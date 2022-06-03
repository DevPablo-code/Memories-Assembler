#pragma once

#include <string>

std::string toLowerCopy(const std::string &s) 
{
	std::string result;

	for (const auto& c : s) 
	{
		result.push_back(tolower(c));
	}

	return result;
}