#include <iostream>
#include <fstream>

#include "Args.h"
#include "BMPImage.h"
#include "BlurBMP.h"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "RU");

    int blurCount = 64;

    std::optional<Args> args = ParseArgs(argc, argv);
    if (!args)
    {
        return EXIT_FAILURE;
    }

    BMPImage image = ReadBMP(args->inputFileName);
    BMPImage buffer1 = image;
    BMPImage buffer2;
    buffer2 = image; 

    for (int i = 0; i < blurCount; i++) 
    {
        if (i % 2 == 0) 
        {
            BlurImage(buffer1, buffer2, args->threadsAmount, args->coresAmount);
        }
        else
        {
            BlurImage(buffer2, buffer1, args->threadsAmount, args->coresAmount);
        }
    }

    BMPImage result = (blurCount % 2 == 0) ? buffer1 : buffer2;

    PrintBMP(args->outputFileName, result);

    return EXIT_SUCCESS;
}