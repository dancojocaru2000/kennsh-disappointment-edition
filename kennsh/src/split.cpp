#include "split.h"

std::vector<std::string> split::split_command(const std::string& input, const std::string& split_at) {
	std::vector<std::string> result;
	std::string buffer;

	bool escape_next = false;
	char escape_until = 0;
	int paren_count = 0;

	for (auto c : input) {
		if (escape_next) {
			buffer += c;
			escape_next = false;
		}
		else if (escape_until != 0) {
			buffer += c;
			if (escape_until == ')' && c == '(') {
				paren_count += 1;
			}
			if (escape_until == c) {
				paren_count -= 1;
			}
			if (paren_count == 0) {
				escape_until = 0;
			}
		}
		else if (c == '`') {
			escape_next = true;
			// Keep ` in the buffer, only remove it in the last pass
			buffer += c;
		}
		else if (c == '\"' || c == '\'') {
			escape_until = c;
			paren_count = 1;
			buffer += c;
		}
		else if (c == '(') {
			escape_until = ')';
			paren_count = 1;
			buffer += c;
		}
		else if (split_at.find_first_of(c) != std::string::npos) {
			result.push_back(buffer);
			buffer = "";
		}
		else {
			buffer += c;
		}
	}

	if (buffer != "") {
		result.push_back(buffer);
	}

	return result;
}