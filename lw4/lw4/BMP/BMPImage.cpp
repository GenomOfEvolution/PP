#include "BMPImage.h"
#include <iostream>
#include <fstream>

BMPImage ReadBMP(const std::string& inputName)
{
    std::ifstream input(inputName, std::ios::binary);
    if (!input)
    {
        throw std::runtime_error("Can't open input file!\n");
    }

    BMPImage image;

    image.fileHeader = GetFileHeader(input);
    image.infoHeader = GetInfoHeader(input);

    const int bytesPerPixel = image.infoHeader.biBitCount / 8;
    const int linePadding = ((image.infoHeader.biWidth * bytesPerPixel) % 4) & 3;

    image.pixels.resize(image.infoHeader.biHeight);

    for (auto& row : image.pixels) 
    {
        row.resize(image.infoHeader.biWidth);
    }
    
    unsigned int buffer;

    for (unsigned int i = 0; i < image.infoHeader.biHeight; i++)
    {
        for (unsigned int j = 0; j < image.infoHeader.biWidth; j++)
        {
            read(input, buffer, bytesPerPixel);

            image.pixels[i][j].rgbRed = BitExtract(buffer, image.infoHeader.biRedMask);
            image.pixels[i][j].rgbGreen = BitExtract(buffer, image.infoHeader.biGreenMask);
            image.pixels[i][j].rgbBlue = BitExtract(buffer, image.infoHeader.biBlueMask);
            image.pixels[i][j].rgbReserved = BitExtract(buffer, image.infoHeader.biAlphaMask);
        }
        input.seekg(linePadding, std::ios_base::cur);
    }

    return image;
}

static int CalculateShift(unsigned int mask) 
{
    if (mask == 0) 
    { 
        return 0; 
    }

    int shift = 0;
    while ((mask & 1) == 0) 
    {
        shift++;
        mask >>= 1;
    }
    return shift;
}


static unsigned int ComposeColor(const ColorRGBA& rgb,
    unsigned int redMask, int redShift,
    unsigned int greenMask, int greenShift,
    unsigned int blueMask, int blueShift,
    unsigned int alphaMask, int alphaShift) 
{
    unsigned int color = 0;

    color |= ((rgb.rgbRed << redShift) & redMask);
    color |= ((rgb.rgbGreen << greenShift) & greenMask);
    color |= ((rgb.rgbBlue << blueShift) & blueMask);
    color |= ((rgb.rgbReserved << alphaShift) & alphaMask);

    return color;
}

void PrintBMP(const std::string& outputName, const BMPImage& image) 
{
    std::ofstream output(outputName, std::ios::binary);
    if (!output) 
    {
        throw std::runtime_error("Can't open output file!\n");
    }

    output.write(reinterpret_cast<const char*>(&image.fileHeader), sizeof(image.fileHeader));
    output.write(reinterpret_cast<const char*>(&image.infoHeader), sizeof(image.infoHeader));

    const int bytesPerPixel = image.infoHeader.biBitCount / 8;
    const int linePadding = ((image.infoHeader.biWidth * bytesPerPixel) % 4) & 3;
    const unsigned int height = image.infoHeader.biHeight;

    int redShift = CalculateShift(image.infoHeader.biRedMask);
    int greenShift = CalculateShift(image.infoHeader.biGreenMask);
    int blueShift = CalculateShift(image.infoHeader.biBlueMask);
    int alphaShift = CalculateShift(image.infoHeader.biAlphaMask);

    for (unsigned int i = 0; i < height; i++) 
    {
        for (unsigned int j = 0; j < image.infoHeader.biWidth; j++) 
        {
            unsigned int color = ComposeColor(
                image.pixels[i][j],
                image.infoHeader.biRedMask, redShift,
                image.infoHeader.biGreenMask, greenShift,
                image.infoHeader.biBlueMask, blueShift,
                image.infoHeader.biAlphaMask, alphaShift
            );

            output.write(reinterpret_cast<const char*>(&color), bytesPerPixel);
        }

        for (int p = 0; p < linePadding; p++) 
        {
            output.put(0);
        }
    }
}