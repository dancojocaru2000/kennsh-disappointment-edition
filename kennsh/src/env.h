#ifndef KENNSH_ENV
#define KENNSH_ENV

#include <string>
#include <vector>

namespace kennsh::command::env {
	uint8_t env(std::vector<std::string> args);
}

#endif