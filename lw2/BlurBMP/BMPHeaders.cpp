#include "BMPHeaders.h"
#include <iostream>

FileHeader GetFileHeader(std::ifstream& input)
{
    FileHeader fileHeader{};
    input.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));

    if (!input)
    {
        throw std::runtime_error("Error reading file header!\n");
    }

    if (fileHeader.bfType != 0x4D42)
    {
        throw std::runtime_error("Input file is not BMP\n");
    }

    return fileHeader;
}

void TryBMPv1(std::ifstream& input, InfoHeader& infoHeader, int bitsOnColor, int maskValue)
{
    if (infoHeader.biSize >= 40) 
    {
        read(input, infoHeader.biCompression, sizeof(infoHeader.biCompression));
        read(input, infoHeader.biSizeImage, sizeof(infoHeader.biSizeImage));
        read(input, infoHeader.biXPelsPerMeter, sizeof(infoHeader.biXPelsPerMeter));
        read(input, infoHeader.biYPelsPerMeter, sizeof(infoHeader.biYPelsPerMeter));
        read(input, infoHeader.biClrUsed, sizeof(infoHeader.biClrUsed));
        read(input, infoHeader.biClrImportant, sizeof(infoHeader.biClrImportant));
    }
}

void TryBMPv2(std::ifstream& input, InfoHeader& infoHeader, int bitsOnColor, int maskValue)
{
    infoHeader.biRedMask = 0;
    infoHeader.biGreenMask = 0;
    infoHeader.biBlueMask = 0;

    if (infoHeader.biSize >= 52) 
    {
        read(input, infoHeader.biRedMask, sizeof(infoHeader.biRedMask));
        read(input, infoHeader.biGreenMask, sizeof(infoHeader.biGreenMask));
        read(input, infoHeader.biBlueMask, sizeof(infoHeader.biBlueMask));
    }

    if (infoHeader.biRedMask == 0 || 
        infoHeader.biGreenMask == 0 || 
        infoHeader.biBlueMask == 0) 
    {
        infoHeader.biRedMask = maskValue << (bitsOnColor * 2);
        infoHeader.biGreenMask = maskValue << bitsOnColor;
        infoHeader.biBlueMask = maskValue;
    }
}

void TryBMPv3(std::ifstream& input, InfoHeader& infoHeader, int bitsOnColor, int maskValue)
{
    if (infoHeader.biSize >= 56)
    {
        read(input, infoHeader.biAlphaMask, sizeof(infoHeader.biAlphaMask));
    }
    else 
    {
        infoHeader.biAlphaMask = maskValue << (bitsOnColor * 3);
    }
}

void TryBMPv4(std::ifstream& input, InfoHeader& infoHeader)
{
    if (infoHeader.biSize >= 108) 
    {
        read(input, infoHeader.biCSType, sizeof(infoHeader.biCSType));
        read(input, infoHeader.biEndpoints, sizeof(infoHeader.biEndpoints));
        read(input, infoHeader.biGammaRed, sizeof(infoHeader.biGammaRed));
        read(input, infoHeader.biGammaGreen, sizeof(infoHeader.biGammaGreen));
        read(input, infoHeader.biGammaBlue, sizeof(infoHeader.biGammaBlue));
    }
}

void TryBMPv5(std::ifstream& input, InfoHeader& infoHeader)
{
    if (infoHeader.biSize >= 124) 
    {
        read(input, infoHeader.biIntent, sizeof(infoHeader.biIntent));
        read(input, infoHeader.biProfileData, sizeof(infoHeader.biProfileData));
        read(input, infoHeader.biProfileSize, sizeof(infoHeader.biProfileSize));
        read(input, infoHeader.biReserved, sizeof(infoHeader.biReserved));
    }
}

bool IsFormatSupported(const InfoHeader& infoHeader)
{
    if (infoHeader.biSize != 12 && infoHeader.biSize != 40 && infoHeader.biSize != 52 &&
        infoHeader.biSize != 56 && infoHeader.biSize != 108 && infoHeader.biSize != 124) 
    {
        return false;
    }

    if (infoHeader.biBitCount != 16 && infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) 
    {
        return false;
    }

    if (infoHeader.biCompression != 0 && infoHeader.biCompression != 3) 
    {
        return false;
    }

    return true;
}

InfoHeader GetInfoHeader(std::ifstream& input)
{
    InfoHeader infoHeader;
    input.read(reinterpret_cast<char*>(&infoHeader.biSize), sizeof(infoHeader.biSize));

    if (infoHeader.biSize >= 12)
    {
        input.read(reinterpret_cast<char*>(&infoHeader.biWidth), sizeof(infoHeader.biWidth));
        input.read(reinterpret_cast<char*>(&infoHeader.biHeight), sizeof(infoHeader.biHeight));
        input.read(reinterpret_cast<char*>(&infoHeader.biPlanes), sizeof(infoHeader.biPlanes));
        input.read(reinterpret_cast<char*>(&infoHeader.biBitCount), sizeof(infoHeader.biBitCount));
    }

    int colorsCount = infoHeader.biBitCount >> 3;
    if (colorsCount < 3) 
    {
        colorsCount = 3;
    }

    int bitsOnColor = infoHeader.biBitCount / colorsCount;
    int maskValue = (1 << bitsOnColor) - 1;

    TryBMPv1(input, infoHeader, bitsOnColor, maskValue);
    TryBMPv2(input, infoHeader, bitsOnColor, maskValue);
    TryBMPv3(input, infoHeader, bitsOnColor, maskValue);
    TryBMPv4(input, infoHeader);
    TryBMPv5(input, infoHeader);

    if (!IsFormatSupported(infoHeader))
    {
        throw std::runtime_error("This .bmp format is not supported!");
    }

    return infoHeader;
}

unsigned char BitExtract(const unsigned int byte, const unsigned int mask)
{
    if (mask == 0)
    {
        return 0;
    }

    int maskBufer = mask;
    int maskPadding = 0;

    while (!(maskBufer & 1)) 
    {
        maskBufer >>= 1;
        maskPadding++;
    }

    return (byte & mask) >> maskPadding;
}
