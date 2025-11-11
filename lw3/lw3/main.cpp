#include <iostream>
#include <fstream>
#include <Windows.h>
#include <mmsystem.h>
#include <vector>

#pragma comment(lib, "winmm.lib") 

const int OPERATIONS_AMOUNT = 20;
const int THREAD_COUNT = 2;

struct ThreadData
{
    int threadNumber;
    HANDLE hFile;       
    DWORD startTime;    
    HANDLE hMutex;     
};

DWORD WINAPI ThreadProc(LPVOID lpParam) 
{
    ThreadData* data = static_cast<ThreadData*>(lpParam);
    char buffer[32];

    for (int i = 0; i < OPERATIONS_AMOUNT; i++) 
    {
        DWORD currentTime = timeGetTime();
        DWORD elapsed = currentTime - data->startTime;

        sprintf_s(buffer, "%d|%lu\n", data->threadNumber, elapsed);

        WaitForSingleObject(data->hMutex, INFINITE);

        DWORD bytesWritten;
        WriteFile(data->hFile, buffer, static_cast<DWORD>(strlen(buffer)), &bytesWritten, NULL);

        ReleaseMutex(data->hMutex);

        volatile double x = 0.0;
        for (int j = 0; j < 100000; j++) 
        {
            x += sin(cos(sin(static_cast<double>(j))));
        }
    }

    return 0;
}

int main()
{
    setlocale(LC_ALL, "Russian");

    HANDLE hFile = CreateFile(
        L"log.txt",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) 
    {
        std::cerr << "Ошибка создания файла: " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
    if (hMutex == NULL) 
    {
        std::cerr << "Ошибка создания мьютекса: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return EXIT_FAILURE;
    }

    std::cout << "Настройте привязку ядер в Диспетчере задач, затем нажмите Enter..." << std::endl;
    std::cin.ignore();

    // Можно не вручную
    HANDLE hProcess = GetCurrentProcess();
    SetProcessAffinityMask(hProcess, 0b1);
    

    DWORD startTime = timeGetTime();
    std::vector<HANDLE> threads(THREAD_COUNT);
    std::vector<ThreadData> threadData(THREAD_COUNT);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        threadData[i] = 
        {
            i + 1,
            hFile,
            startTime,
            hMutex
        };

        threads[i] = CreateThread(
            NULL,
            0,
            ThreadProc,
            &threadData[i],
            0,
            NULL
        );

        if (threads[i] == NULL) 
        {
            std::cerr << "Ошибка создания потока " << i + 1 << ": " << GetLastError() << std::endl;
            CloseHandle(hFile);
            CloseHandle(hMutex);
            return EXIT_FAILURE;
        }
    }

    // Высокий приоритет для первого потока
    SetThreadPriority(threads[0], THREAD_PRIORITY_HIGHEST);

    WaitForMultipleObjects(THREAD_COUNT, threads.data(), TRUE, INFINITE);

    for (auto& h : threads) 
        CloseHandle(h);

    CloseHandle(hMutex);
    CloseHandle(hFile);

    return EXIT_SUCCESS;
}