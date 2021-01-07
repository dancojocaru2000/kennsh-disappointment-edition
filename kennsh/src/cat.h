#ifndef KENNSH_CAT
#define KENNSH_CAT

#include <vector>
#include <string>

namespace kennsh {
	namespace command {
		namespace cat {
			uint8_t cat(const std::vector<std::string>& args);
		}
	}
}

#endif