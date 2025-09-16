#pragma once
#include <vector>
#include <string>

#include "BMPHeaders.h"

struct BMPImage
{
	FileHeader fileHeader{};
	InfoHeader infoHeader{};
	std::vector<std::vector<ColorRGBA>> pixels;
};

BMPImage ReadBMP(const std::string& inputName);
void PrintBMP(const std::string& outputName, const BMPImage& image);