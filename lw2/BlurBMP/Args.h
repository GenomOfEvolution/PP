#pragma once
#include <string>
#include <optional>

struct Args
{
	std::string inputFileName;
	std::string outputFileName;
	int threadsAmount = 0;
	int coresAmount = 0;
};

std::optional<Args> ParseArgs(int argc, char* argv[]);