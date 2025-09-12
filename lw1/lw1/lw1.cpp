#include <iostream>
#include <string>
#include <Windows.h>
#include <tchar.h>
#include <exception>
#include <vector>

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    int threadNumber = *(int*)lpParam;
    std::cout << "Поток #" << threadNumber << " выполняет свою работу\n";
    return EXIT_SUCCESS;
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "RU");

    if (argc != 2)
    {
        std::cout << "Usage: lw1.exe <threads number>\n";
        return EXIT_FAILURE;
    }

    try
    {
        int threadsAmount = std::stoi(argv[1]);
        std::vector<HANDLE> handles(threadsAmount);
        std::vector<int> threadNumbers(threadsAmount);

        for (int i = 0; i < threadsAmount; i++)
        {
            threadNumbers[i] = i + 1;
            handles[i] = CreateThread(NULL, 0, &ThreadProc, &threadNumbers[i], 0, NULL);
        }

        WaitForMultipleObjects(threadsAmount, handles.data(), TRUE, INFINITE);
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}