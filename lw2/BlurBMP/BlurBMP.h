#pragma once
#include "BMPImage.h"

void BlurImage(const BMPImage& source, BMPImage& dest, int threadsCount, int coresCount);