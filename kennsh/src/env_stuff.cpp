#include "env_stuff.h"

// std::transform
#include <algorithm>
extern "C" {
	// getenv
	#include <stdlib.h>
}

#include "trim.h"

bool kennsh::env_is_true(const std::string& key) {
	const char* value_c = getenv(key.c_str());
	if (value_c == nullptr) {
		return false;
	}
	std::string value(value_c);
	trim::trim(value);
	std::transform(
		value.begin(),
		value.end(),
		value.begin(),
		[](unsigned char c) {
			if ('A' <= c && c <= 'Z') {
				return (unsigned char)(c - 'A' + 'a');
			}
			return c;
		}
	);

	if (value == "") {
		return false;
	}
	else if (value == "0") {
		return false;
	}
	else if (value == "f") {
		return false;
	}
	else if (value == "false") {
		return false;
	}
	else if (value == "n") {
		return false;
	}
	else if (value == "no") {
		return false;
	}
	else {
		return true;
	}
}