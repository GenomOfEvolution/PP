#pragma once
#include "../BMP/BMPImage.h"
#include "../Args/Args.h"

void BlurImage(
	const BMPImage& source,
	BMPImage& dest,
	int threadsCount,
	int coresCount,
	const std::vector<ThreadPriority>& priorities);