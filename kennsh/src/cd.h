#ifndef KENNSH_COMMAND_CD
#define KENNSH_COMMAND_CD

#include <vector>
#include <string>

namespace kennsh::command::cd {
	uint8_t cd(std::vector<std::string> args);
}

#endif