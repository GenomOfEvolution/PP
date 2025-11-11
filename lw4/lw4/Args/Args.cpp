#include <iostream>
#include <sstream>
#include "Args.h"

ThreadPriority ParsePriority(const std::string& str) 
{
    if (str == "normal")
        return ThreadPriority::Normal;
    if (str == "above_normal") 
        return ThreadPriority::AboveNormal;
    return ThreadPriority::BelowNormal;
}

std::optional<Args> ParseArgs(int argc, char* argv[])
{
    if (argc != 6)
    {
        std::cout << "Usage: <InputFile.bmp> <OutputFile.bmp> <ThreadsAmount> <CoreAmount> <ThreadPriorities>\n";
        std::cout << "ThreadPriorities format: priority1,priority2,... (e.g., normal,above_normal,below_normal)\n";
        return std::nullopt;
    }

    Args result;
    result.inputFileName = argv[1];
    result.outputFileName = argv[2];
    result.threadsAmount = std::stoi(argv[3]);
    result.coresAmount = std::stoi(argv[4]);

    std::string prioritiesStr = argv[5];
    std::istringstream ss(prioritiesStr);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        result.threadPriorities.push_back(ParsePriority(token));
    }

    if (result.threadPriorities.size() != static_cast<size_t>(result.threadsAmount))
    {
        std::cout << "Error: Number of priorities (" << result.threadPriorities.size()
            << ") must match thread count (" << result.threadsAmount << ")\n";
        return std::nullopt;
    }

    return result;
}