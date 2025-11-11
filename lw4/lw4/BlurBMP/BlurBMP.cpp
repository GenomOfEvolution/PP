#define NOMINMAX
#include "BlurBMP.h"
#include <algorithm>
#include <vector>
#include <Windows.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

struct Block
{
	int startX = 0;
	int startY = 0;
	int endX = 0;
	int endY = 0;
};

struct ThreadData
{
	const BMPImage* source{};
	BMPImage* dest{};
	std::vector<Block> blocks;
	int blurRadius = 0;
	int threadIndex = -1;
	FILE* logFile = nullptr;
	HANDLE logMutex = nullptr;
	DWORD startProgramTime = 0;
};

static std::vector<Block> SplitOnBlocks(const BMPImage& source, int threadsCount)
{
	const int BLUR_RADIUS = 1;

	const int width = source.infoHeader.biWidth;
	const int height = source.infoHeader.biHeight;

	const int blocksX = threadsCount;
	const int blocksY = threadsCount;

	const int blockWidth = (int)std::ceil((float)width / blocksX);
	const int blockHeight = (int)std::ceil((float)height / blocksY);

	std::vector<Block> result;

	for (int y = 0; y < blocksY; y++) 
	{
		for (int x = 0; x < blocksX; x++) 
		{
			Block block;
			block.startX = std::max(0, x * blockWidth - BLUR_RADIUS);
			block.startY = std::max(0, y * blockHeight - BLUR_RADIUS);
			block.endX = std::min(width, (x + 1) * blockWidth + BLUR_RADIUS);
			block.endY = std::min(height, (y + 1) * blockHeight + BLUR_RADIUS);

			result.push_back(block);
		}
	}

	return result;
}

static bool ComputeBlurredPixel(const BMPImage& source, int x, int y, int blurRadius, ColorRGBA& resultPixel)
{
	int rSum = 0, gSum = 0, bSum = 0, aSum = 0;
	int count = 0;

	for (int dy = -blurRadius; dy <= blurRadius; dy++)
	{
		for (int dx = -blurRadius; dx <= blurRadius; dx++)
		{
			int nx = x + dx;
			int ny = y + dy;

			if (nx >= 0 && nx < static_cast<int>(source.infoHeader.biWidth) &&
				ny >= 0 && ny < static_cast<int>(source.infoHeader.biHeight))
			{
				const ColorRGBA& pixel = source.pixels[ny][nx];
				rSum += pixel.rgbRed;
				gSum += pixel.rgbGreen;
				bSum += pixel.rgbBlue;
				aSum += pixel.rgbReserved;
				count++;
			}
		}
	}

	if (count == 0)
		return false;

	resultPixel.rgbRed = static_cast<BYTE>(rSum / count);
	resultPixel.rgbGreen = static_cast<BYTE>(gSum / count);
	resultPixel.rgbBlue = static_cast<BYTE>(bSum / count);
	resultPixel.rgbReserved = static_cast<BYTE>(aSum / count);
	return true;
}

DWORD WINAPI BlurThread(CONST LPVOID lpParam)
{
	ThreadData* data = (ThreadData*)lpParam;
	const BMPImage& source = *(data->source);
	BMPImage& dest = *(data->dest);
	int blurRadius = data->blurRadius;

	for (const auto& block : data->blocks)
	{
		int innerStartX = std::max(block.startX + blurRadius, 0);
		int innerEndX = std::min(block.endX - blurRadius, static_cast<int>(source.infoHeader.biWidth));
		int innerStartY = std::max(block.startY + blurRadius, 0);
		int innerEndY = std::min(block.endY - blurRadius, static_cast<int>(source.infoHeader.biHeight));

		if (!(innerStartX < innerEndX && innerStartY < innerEndY))
		{
			return EXIT_FAILURE;
		}

		for (int y = innerStartY; y < innerEndY; y++)
		{
			for (int x = innerStartX; x < innerEndX; x++)
			{
				ColorRGBA resultPixel;
				bool blurPixelResult = true;

				// NOTE: делаем много раз, чтобы замедлить
				for (int i = 0; i < 1024; i++)
				{
					blurPixelResult = ComputeBlurredPixel(source, x, y, blurRadius, resultPixel);
				}

				dest.pixels[y][x] = resultPixel;

				if (data->logFile && data->logMutex)
				{
					WaitForSingleObject(data->logMutex, INFINITE);
					fprintf(data->logFile, "%d,%u\n", data->threadIndex, timeGetTime() - data->startProgramTime);
					ReleaseMutex(data->logMutex);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

void BlurImage(
	const BMPImage& source,
	BMPImage& dest,
	int threadsCount,
	int coresCount,
	const std::vector<ThreadPriority>& priorities)
{
	DWORD startProgramTime = timeGetTime();

	FILE* logFile = nullptr;
	errno_t err = fopen_s(&logFile, "blur_log.csv", "w");

	if (logFile) {
		fprintf(logFile, "thread_id,duration_ms\n");
		fclose(logFile);
	}

	err = fopen_s(&logFile, "blur_log.csv", "a");
	if (err != 0) 
	{
		logFile = nullptr; 
	}

	HANDLE logMutex = CreateMutex(NULL, FALSE, NULL);

	auto blocks = SplitOnBlocks(source, threadsCount);
	std::vector<std::vector<Block>> threadBlocks(threadsCount);

	for (int i = 0; i < blocks.size(); i++)
	{
		threadBlocks[i % threadsCount].push_back(blocks[i]);
	}

	dest = source;
	std::vector<ThreadData> threadData(threadsCount);
	std::vector<HANDLE> handles(threadsCount);

	DWORD_PTR affinityMask = 0;
	for (int i = 0; i < coresCount; i++)
	{
		affinityMask |= (1ULL << i);
	}

	for (int i = 0; i < threadsCount; i++)
	{
		threadData[i].source = &source;
		threadData[i].dest = &dest;
		threadData[i].blocks = threadBlocks[i];
		threadData[i].blurRadius = 1;
		threadData[i].threadIndex = i + 1;
		threadData[i].logFile = logFile;
		threadData[i].logMutex = logMutex;
		threadData[i].startProgramTime = startProgramTime;

		handles[i] = CreateThread(NULL, 0, &BlurThread, &threadData[i], CREATE_SUSPENDED, NULL);
		SetThreadAffinityMask(handles[i], affinityMask);

		int winPriority = THREAD_PRIORITY_NORMAL;
		switch (priorities[i])
		{
		case ThreadPriority::AboveNormal:
			winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case ThreadPriority::BelowNormal:
			winPriority = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case ThreadPriority::Normal:
			winPriority = THREAD_PRIORITY_NORMAL;
			break;
		}
		SetThreadPriority(handles[i], winPriority);
	}

	for (auto handle : handles)
	{
		ResumeThread(handle);
	}

	WaitForMultipleObjects((DWORD)handles.size(), handles.data(), TRUE, INFINITE);

	for (auto handle : handles)
	{
		CloseHandle(handle);
	}

	if (logFile) fclose(logFile);
	if (logMutex) CloseHandle(logMutex);
}
