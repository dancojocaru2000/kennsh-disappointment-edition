// int8_t and the like
#include <cstdint>
#include <string>
#include <iostream>

extern "C" {
	// linenoise library for history and prompt
	#include <stddef.h>
	#include <linenoise.h>
	// setenv, getenv
	#include <stdlib.h>
	// isatty
	#include <unistd.h>
	#include <errno.h>

	// Hacky hack: hijack functions from linenoise
	int enableRawMode(int fd);
	void disableRawMode(int fd);
	int getCursorPosition(int ifd, int ofd);
}

#include "file_stuff.h"
#include "c_error.h"
#include "env_stuff.h"
#include "trim.h"
#include "command.h"
#include "error.h"

std::string prompt(bool error) {
	auto use_powerline = kennsh::env_is_true("use_powerline");
	// TODO: Powerline
	// else {
		std::string result;
		result += "\x1b[";
		if (error) { result += "41"; } else { result += "7"; }
		result += "m";
		result += "[kennsh]";
		// TODO: Path
		result += " >";
		result += "\x1b[0m";
		result += " ";
		return result;
	//}
}
std::string rawprompt(bool error) {
	auto use_powerline = kennsh::env_is_true("use_powerline");
	// TODO: Powerline
	// else {
		std::string result;
		// result += "\x1b[";
		// if (error) { result += "41"; } else { result += "7"; }
		// result += "m";
		result += "[kennsh]";
		// TODO: Path
		result += " ";
		auto old_value = getenv("who_is_running");
		if (old_value != nullptr) {
			setenv("who_is_running", "0", 1);
		}
		result += trim::trim_copy(kennsh::command::run_subcommand("pwd"));
		if (old_value != nullptr) {
			setenv("who_is_running", old_value, 1);
		}
		result += " >";
		// result += "\x1b[0m";
		result += " ";
		return result;
	//}
}

int main() {
	// Initialize linenoise
	linenoiseHistorySetMaxLen(100);

	uint8_t last_exit_code = 0;

	while (true) {
		// Set status variable
		kennsh::ewrap(setenv("status", std::to_string(last_exit_code).c_str(), 1));

		// TODO: Set window title

		// TODO: Check if at beginning of line, if not output newline
		if (isatty(kennsh::STDIN_FD)) {
			enableRawMode(kennsh::STDIN_FD);
			auto cp = getCursorPosition(kennsh::STDIN_FD, kennsh::STDOUT_FD);
			disableRawMode(kennsh::STDIN_FD);
			// std::cerr << "cp: " << cp << std::endl;
			if (cp != 1) {
				std::cout << "\x1b[7m⏎\x1b[27m" << std::endl;
			}
		}

		// If some application would forget to reset stdin to blocking, reset it
		try {
			kennsh::set_nonblocking(kennsh::STDIN_FD, false);
		}
		catch (kennsh::c_error& e) {}
	
		char* line_c = linenoise(
			rawprompt(last_exit_code != 0).c_str()
		);
		if (line_c == nullptr) {
			break;
		}
		std::string line(line_c);
		linenoiseFree(line_c);
		trim::trim(line);
		if (line != "") {
			linenoiseHistoryAdd(line.c_str());
			try {
				uint8_t errorcode = kennsh::command::handle(line);
				last_exit_code = errorcode;
			}
			catch (kennsh::exit_request& er) {
				if (er.use_last_status_code) {
					exit(last_exit_code);
				}
				else {
					exit(er.exit_code);
				}
			}
			catch (kennsh::command_exception& e) {
				std::cerr << e.what() << std::endl;
				last_exit_code = kennsh::ec::OTHER_ERROR;
			}
			catch (kennsh::c_error& e) {
				if (e.num == ENOENT) {
					last_exit_code = kennsh::ec::FILE_NOT_FOUND;
				}
				else if (e.num == EACCES) {
					last_exit_code = kennsh::ec::PERMISSION_DENIED;
				}
				else {
					std::cerr << e.what() << std::endl;
					last_exit_code = kennsh::ec::OTHER_ERROR;
				}
			}

			if (last_exit_code == 127) {
				std::cerr << "kennsh: File or directory not found" << std::endl;
			}
			else if (last_exit_code == 126) {
				std::cerr << "kennsh: Permission denied" << std::endl;
			}
			else if (last_exit_code != 0) {
				std::cerr << "kennsh: Program exited with code " << (int)last_exit_code << std::endl;
			}
		}
	}

	return 0;
}