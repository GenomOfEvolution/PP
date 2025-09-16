#define NOMINMAX
#include "BlurBMP.h"
#include <algorithm>
#include <vector>
#include <Windows.h>

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
};

std::vector<Block> SplitOnBlocks(const BMPImage& source, int threadsCount)
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

DWORD WINAPI BlurThread(CONST LPVOID lpParam) 
{
	ThreadData* data = (ThreadData*)lpParam;
	const BMPImage& source = *(data->source);
	BMPImage& dest = *(data->dest);
	int blurRadius = data->blurRadius;

	for (const auto& block : data->blocks)
	{
		int innerStartX = std::max(block.startX + blurRadius, 0);
		int innerEndX = std::min(block.endX - blurRadius, (int)source.infoHeader.biWidth);
		int innerStartY = std::max(block.startY + blurRadius, 0);
		int innerEndY = std::min(block.endY - blurRadius, (int)source.infoHeader.biHeight);

		if (!(innerStartX < innerEndX && innerStartY < innerEndY))
		{
			return EXIT_FAILURE;
		}

		for (int y = innerStartY; y < innerEndY; y++)
		{
			for (int x = innerStartX; x < innerEndX; x++)
			{
				int rSum = 0, gSum = 0, bSum = 0, aSum = 0;
				int count = 0;

				for (int dy = -blurRadius; dy <= blurRadius; dy++)
				{
					for (int dx = -blurRadius; dx <= blurRadius; dx++)
					{
						int nx = x + dx;
						int ny = y + dy;

						if (nx >= 0 && nx < source.infoHeader.biWidth &&
							ny >= 0 && ny < source.infoHeader.biHeight)
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
					continue;

				ColorRGBA resultPixel;

				resultPixel.rgbBlue = bSum / count;
				resultPixel.rgbRed = rSum / count;
				resultPixel.rgbGreen = gSum / count;
				resultPixel.rgbReserved = aSum / count;

				dest.pixels[y][x] = resultPixel;
			}
		}
	}

	return EXIT_SUCCESS;
}

void BlurImage(const BMPImage& source, BMPImage& dest, int threadsCount, int coresCount)
{
	auto blocks = SplitOnBlocks(source, threadsCount);
	std::vector<std::vector<Block>> threadBlocks(threadsCount);

	for (int i = 0; i < blocks.size(); i++)
	{
		threadBlocks[i % threadsCount].push_back(blocks[i]);
	}

	dest = source;

	std::vector<ThreadData> threadData(threadsCount);
	std::vector<HANDLE> handles(threadsCount);

	for (int i = 0; i < threadsCount; i++)
	{
		threadData[i].source = &source;
		threadData[i].dest = &dest;
		threadData[i].blocks = threadBlocks[i];
		threadData[i].blurRadius = 1;

		handles[i] = CreateThread(NULL, 0, &BlurThread, &threadData[i], CREATE_SUSPENDED, NULL);
	}

	for (auto handle : handles) 
	{
		ResumeThread(handle);
	}

	WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);

	for (auto handle : handles) 
	{
		CloseHandle(handle);
	}
}
