#pragma once
#include <fstream>

#pragma pack(push, 1)
struct FileHeader
{
    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MyCIEXYZ
{
    int ciexyzX;
    int ciexyzY;
    int ciexyzZ;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MyCIEXYZTRIPLE
{
    MyCIEXYZ ciexyzRed;
    MyCIEXYZ ciexyzGreen;
    MyCIEXYZ ciexyzBlue;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct InfoHeader
{
    unsigned int   biSize;
    unsigned int   biWidth;
    unsigned int   biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    unsigned int   biXPelsPerMeter;
    unsigned int   biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
    unsigned int   biRedMask;
    unsigned int   biGreenMask;
    unsigned int   biBlueMask;
    unsigned int   biAlphaMask;
    unsigned int   biCSType;
    MyCIEXYZTRIPLE biEndpoints;
    unsigned int   biGammaRed;
    unsigned int   biGammaGreen;
    unsigned int   biGammaBlue;
    unsigned int   biIntent;
    unsigned int   biProfileData;
    unsigned int   biProfileSize;
    unsigned int   biReserved;
};
#pragma pack(pop)

struct ColorRGBA 
{
    unsigned char  rgbBlue = 0;
    unsigned char  rgbGreen = 0;
    unsigned char  rgbRed = 0;
    unsigned char  rgbReserved = 0;
};

FileHeader GetFileHeader(std::ifstream& input);
InfoHeader GetInfoHeader(std::ifstream& input);
unsigned char BitExtract(const unsigned int byte, const unsigned int mask);

template <typename Type>
void read(std::ifstream& fp, Type& result, std::size_t size)
{
    fp.read(reinterpret_cast<char*>(&result), size);
}