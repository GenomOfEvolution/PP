#pragma once
#include <string>
#include <optional>
#include <vector>

enum class ThreadPriority 
{
    Normal,
    AboveNormal,
    BelowNormal
};

struct Args
{
    std::string inputFileName;
    std::string outputFileName;
    int threadsAmount = 0;
    int coresAmount = 0;
    std::vector<ThreadPriority> threadPriorities;
};

std::optional<Args> ParseArgs(int argc, char* argv[]);