#include "env.h"

#include <iostream>
// std::pair
#include <utility>
extern "C" {
	// isatty, fork
	#include <unistd.h>
	// fork, waitpid
	#include <sys/types.h>
	// waitpid
	#include <sys/wait.h>
	// setenv, unsetenv
	#include <stdlib.h>
}

#include "command.h"
#include "file_stuff.h"
#include "env_stuff.h"
#include "c_error.h"

namespace kennsh::command::env {
	std::vector<std::pair<std::string, std::string>> get_full_environment() {
		std::vector<std::pair<std::string, std::string>> result;
		for (char** p = environ; *p; p++) {
			std::string s(*p);
			auto eq_pos = s.find('=');
			result.emplace_back(s.substr(0, eq_pos), s.substr(eq_pos + 1));
		}
		return result;
	}

	uint8_t env(std::vector<std::string> args) {
		if (args.size() == 1) {
			// Print the environment
			bool print_color = isatty(STDOUT_FD);
			if (print_color) {
				print_color = !env_is_true("NOCOLOR");
			}
			if (print_color) {
				print_color = !env_is_true("NO_COLOR");
			}

			for (const auto& p : get_full_environment()) {
				if (print_color) {
					std::cout << "\x1b[94m";
				}
				std::cout << p.first;
				if (print_color) {
					std::cout << "\x1b[39m";
				}
				std::cout << "=";
				if (print_color) {
					std::cout << "\x1b[92m";
				}
				std::cout << p.second;
				if (print_color) {
					std::cout << "\x1b[39m";
				}
				std::cout << std::endl;
			}

			return 0;
		}

		// Fork then edit environment
		auto child_pid = ewrap(fork());
		if (child_pid == 0) {

			args.erase(args.begin());
			std::vector<std::string> command;

			auto it = args.begin();
			// Fisrt, handle -u and -i, A=B
			for (;it != args.end(); it++) {
				auto& arg = *it;
				if (arg == "-i") {
					// Reset environment
					for (const auto& p : get_full_environment()) {
						unsetenv(p.first.c_str());
					}
				}
				else if (arg == "-u") {
					it++;
					if (it == args.end()) {
						// TODO: Error
					}
					unsetenv((*it).c_str());
				}
				else if (arg[0] == '-') {
					// TODO: Error
				}
				else if (arg.find('=') != std::string::npos) {
					auto eq_pos = arg.find('=');
					setenv(arg.substr(0, eq_pos).c_str(), arg.substr(eq_pos + 1).c_str(), 1);
				}
				else {
					break;
				}
			}
			for (; it != args.end(); it++) {
				command.push_back(*it);
			}
			if (command.size() == 0) {
				command.push_back("env");
			}
			auto result = process_command(command);
			exit(result);
		}
		else {
			int status;
			ewrap(waitpid(child_pid, &status, 0));
			return WEXITSTATUS(status);
		}
	}

	uint8_t set(std::vector<std::string> args) {
		args.erase(args.begin());
		if (args.size() != 2) {
			throw command_exception("set: 2 arguments are required");
		}
		setenv(args[0].c_str(), args[1].c_str(), 1);
		return 0;
	}

	uint8_t unset(std::vector<std::string> args) {
		args.erase(args.begin());
		if (args.size() != 1) {
			throw command_exception("unset: 1 argument is required");
		}
		unsetenv(args[0].c_str());
		return 0;
	}
}