#include <iostream>
#include <fstream>
#include <chrono>

#include "Args/Args.h"
#include "BMP/BMPImage.h"
#include "BlurBMP/BlurBMP.h"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "RU");

    std::optional<Args> args = ParseArgs(argc, argv);
    if (!args)
    {
        return EXIT_FAILURE;
    }

    auto start = std::chrono::high_resolution_clock::now();

    BMPImage image = ReadBMP(args->inputFileName);
    BMPImage buffer1 = image;
    BMPImage buffer2 = image;

    int blurCount = 0;
    const int MAX_ITERATIONS = 1;

    while (true)
    {
        if (blurCount % 2 == 0)
        {
            BlurImage(buffer1, buffer2, args->threadsAmount, args->coresAmount, args->threadPriorities);
        }
        else
        {
            BlurImage(buffer2, buffer1, args->threadsAmount, args->coresAmount, args->threadPriorities);
        }

        blurCount++;

        auto current = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current - start;


        if (elapsed.count() >= 0.5)
        {
            break;
        }

        if (blurCount >= MAX_ITERATIONS)
        {
            break;
        }
    }

    BMPImage result = (blurCount % 2 == 0) ? buffer1 : buffer2;
    PrintBMP(args->outputFileName, result);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Blur applied " << blurCount << " times." << std::endl;
    std::cout << "Threads: " << args->threadsAmount << "; Cores: " << args->coresAmount << std::endl;
    std::cout << "Total time: " << elapsed.count() << " seconds." << std::endl;

    return EXIT_SUCCESS;
}