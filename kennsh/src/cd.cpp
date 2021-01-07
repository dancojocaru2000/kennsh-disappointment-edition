#include "cd.h"

extern "C" {
	// chdir
	#include <unistd.h>
	#include <errno.h>
}

#include "command.h"
#include "c_error.h"

namespace kennsh::command::cd {
	uint8_t cd(std::vector<std::string> args) {
		args.erase(args.begin());
		if (args.size() != 1) {
			throw command_exception("cd: 1 argument is required");
		}
		int res = chdir(args[0].c_str());
		if (res == -1) {
			if (errno == EACCES) {
				return 126;
			} 
			else if (errno == ENOENT) {
				return 127;
			}
			else {
				ewrap(-1);
			}
		}
		return 0;
	}	
}