#ifndef SPLIT_SPLIT
#define SPLIT_SPLIT

#include <vector>
#include <string>

namespace split {
	std::vector<std::string> split_command(const std::string& input, const std::string& split_for);
	std::vector<std::string> split_command(const std::string& input, const std::string& split_for, const std::string& skip);
}

#endif