#include "trim.h"

// std::find_if
#include <algorithm>
// std::isspace
#include <cctype>

void trim::ltrim(std::string& s) {
	s.erase(
		s.begin(),
		std::find_if(
			s.begin(),
			s.end(),
			[](unsigned char c) {
				return !std::isspace(c);
			}
		)
	);
}

void trim::rtrim(std::string& s) {
	s.erase(
		std::find_if(
			s.rbegin(),
			s.rend(),
			[](unsigned char c) {
				return !std::isspace(c);
			}
		).base(),
		s.end()
	);
}

void trim::trim(std::string& s) {
	trim::ltrim(s);
	trim::rtrim(s);
}

std::string trim::ltrim_copy(std::string s) {
	trim::ltrim(s);
	return s;
}
std::string trim::rtrim_copy(std::string s) {
	trim::rtrim(s);
	return s;
}
std::string trim::trim_copy(std::string s) {
	trim::trim(s);
	return s;
}