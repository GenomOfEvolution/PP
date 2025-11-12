#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>

HANDLE GlobalBalanceMutex = NULL;
CRITICAL_SECTION ThreadBalanceCriticalSection;

const LPCWSTR GLOBAL_MUTEX_NAME = L"Global\\BankBalanceMutex_2025";

int ReadBalance()
{
    WaitForSingleObject(GlobalBalanceMutex, INFINITE);

    std::fstream file("balance.txt", std::ios_base::in);
    int balance = 0;
    if (file.is_open()) 
    {
        file >> balance;
    }
    file.close();

    ReleaseMutex(GlobalBalanceMutex);
    return balance;
}

void Deposit(int amount)
{
    WaitForSingleObject(GlobalBalanceMutex, INFINITE);

    std::fstream file("balance.txt", std::ios_base::in | std::ios_base::out);
    int balance = 0;
    if (file.is_open()) 
    {
        file >> balance;
        balance += amount;

        file.seekp(0);
        file << balance;
        printf("[PID:%d] Deposited %d. New balance: %d\n",
            GetCurrentProcessId(), amount, balance);
    }
    file.close();

    ReleaseMutex(GlobalBalanceMutex);
}

bool Withdraw(int amount)
{
    WaitForSingleObject(GlobalBalanceMutex, INFINITE);

    std::fstream file("balance.txt", std::ios_base::in | std::ios_base::out);
    int balance = 0;
    bool success = false;

    if (file.is_open())
    {
        file >> balance;

        if (balance >= amount) 
        {
            balance -= amount;
            file.seekp(0);
            file << balance;
            printf("[PID:%d] Withdrawn %d. New balance: %d\n",
                GetCurrentProcessId(), amount, balance);
            success = true;
        }
        else 
        {
            printf("[PID:%d] FAILED to withdraw %d. Balance: %d\n",
                GetCurrentProcessId(), amount, balance);
        }
    }
    file.close();

    ReleaseMutex(GlobalBalanceMutex);
    return success;
}

DWORD WINAPI DoDeposit(LPVOID lpParameter)
{
    EnterCriticalSection(&ThreadBalanceCriticalSection);
    Deposit((int)lpParameter);
    LeaveCriticalSection(&ThreadBalanceCriticalSection);
    return 0;
}

DWORD WINAPI DoWithdraw(LPVOID lpParameter)
{
    EnterCriticalSection(&ThreadBalanceCriticalSection);
    Withdraw((int)lpParameter);
    LeaveCriticalSection(&ThreadBalanceCriticalSection);
    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    GlobalBalanceMutex = CreateMutex(
        NULL, FALSE, GLOBAL_MUTEX_NAME
    );

    if (!GlobalBalanceMutex) 
    {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }

    if (GetLastError() != ERROR_ALREADY_EXISTS) 
    {
        std::ofstream initFile("balance.txt");
        initFile << "0" << std::endl;
        initFile.close();
        printf("[PID:%d] Initialized balance to 0\n", GetCurrentProcessId());
    }

    InitializeCriticalSection(&ThreadBalanceCriticalSection);

    HANDLE threads[50];
    SetProcessAffinityMask(GetCurrentProcess(), 1);

    for (int i = 0; i < 50; i++) 
    {
        DWORD threadId;
        threads[i] = CreateThread(
            NULL, 0,
            (i % 2 == 0) ? DoDeposit : DoWithdraw,
            (LPVOID)(i % 2 == 0 ? 230 : 1000),
            0, &threadId
        );
    }

    WaitForMultipleObjects(50, threads, TRUE, INFINITE);

    int finalBalance = ReadBalance();
    printf("[PID:%d] FINAL BALANCE: %d\n", GetCurrentProcessId(), finalBalance);

    for (int i = 0; i < 50; i++)
        CloseHandle(threads[i]);

    DeleteCriticalSection(&ThreadBalanceCriticalSection);
    CloseHandle(GlobalBalanceMutex);

    getchar();
    return 0;
}