#include <iostream>

#include "Args.h"
#include "BMPHeaders.h"

std::optional<Args> ParseArgs(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cout << "Usage: <InputFile.bmp> <OutputFile.bmp> <ThreadsAmount> <CoreAmount>\n";
        return std::nullopt;
    }

    Args result;
    result.inputFileName = argv[1];
    result.outputFileName = argv[2];

    result.threadsAmount = std::stoi(argv[3]);
    result.coresAmount = std::stoi(argv[4]);

    return result;
}
