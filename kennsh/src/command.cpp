#include "command.h"

#include <iostream>
// std::transform
#include <algorithm>
// stringstream => convert from string to int
#include <sstream>

extern "C" {
	#include <sys/types.h>
	#include <unistd.h>
	#include <sys/wait.h>
	#include <stdlib.h>
	#include <errno.h>
	// for open
	#include <sys/stat.h>
	#include <fcntl.h>
}

#include "split.h"
#include "trim.h"
#include "file_stuff.h"
#include "c_error.h"
#include "env_stuff.h"
#include "cat.h"

namespace kennsh {
	namespace command {
		uint8_t exit_command(const std::vector<std::string>& args) {
			if (args.size() == 1) {
				throw exit_request();
			}
			else if (args.size() == 2) {
				int tmp;
				std::stringstream ss(args[1]);
				ss >> tmp;
				throw(exit_request((uint8_t)tmp));
			}
			else {
				std::cerr << "exit: Wrong number of arguments provided: " << args.size() - 1 << std::endl;
				std::cerr << "      Extra arguments will be ignored." << std::endl;
				return exit_command(std::vector<std::string>(args.begin(), args.begin() + 2));
			}
		}

		uint8_t external_command(const std::vector<std::string>& args) {
			// Convert to C style array
			typedef char* CString;
			
			// Fork
			auto child_pid = ewrap(fork());
			if (child_pid == 0) {
				// Child
				std::vector<CString> c_args(args.size() + 1, nullptr);
				for (int i = 0; i < args.size(); i++) {
					c_args[i] = (char*)args[i].c_str();
				}
				execvp(c_args[0], c_args.data());
				if (errno == ENOENT) {
					exit(127);
				}
				else if (errno == EACCES) {
					exit(126);
				}
				exit(1);
			}
			else {
				// Parent
				int status;
				ewrap(waitpid(child_pid, &status, 0));
				return WEXITSTATUS(status);
			}
		}

		std::string run_subcommand(const std::string& subcommand) {
			// Steps:
			// 1. Create pipe for stdout to return
			// 2. Create closed pipe for stdin (stdin for subcommand
			//    should have no data)
			// 3. Fork
			int stdout_pipe[2];
			ewrap(pipe(stdout_pipe));
			int stdin_pipe[2];
			ewrap(pipe(stdin_pipe));
			ewrap(close(stdin_pipe[1]));

			pid_t child_pid = ewrap(fork());
			if (child_pid == 0) {
				// Child
				ewrap(dup2(stdin_pipe[0], STDIN_FD));
				ewrap(close(stdin_pipe[0]));
				ewrap(dup2(stdout_pipe[1], STDOUT_FD));
				ewrap(close(stdout_pipe[1]));
				auto result = handle(subcommand);
				exit(result);
			}
			else {
				// Parent
				// Read into string until pipe is closed
				ewrap(close(stdout_pipe[1]));
				std::string result;
				char buffer[4097];
				buffer[4096] = 0;
				while (true) {
					auto bytes_read = ewrap(read(stdout_pipe[0], buffer, 4096));
					if (bytes_read == 0) {
						break;
					}
					buffer[bytes_read] = 0;
					result += std::string(buffer);
				}
				return result;
			}
		}

		std::string process_argument(const std::string& arg) {
			std::string result;
			std::string buffer;
			const int M_RAW_STRING = 0;
			const int M_STRING = 1;
			const int M_SHELL_VARIABLE = 2;
			const int M_SUBCOMMAND = 3;
			int mode = 0;
			int subcommand_depth = 0;
			bool escape_next = false;
			char string_char = 0;

			for (auto c : arg) {
				if (mode == M_RAW_STRING) {
					if (escape_next) {
						escape_next = false;
						// TODO: Escape more?
						if (c == 'n') {
							buffer += '\n';
						}
						else if (c == 'r') {
							buffer += '\r';
						}
						else if (c == 'e') {
							buffer += '\x1b';
						}
						else if (c == 't') {
							buffer += '\t';
						}
						else {
							buffer += c;
						}
					}
					else if (c == '`') {
						escape_next = true;
					}
					else if (c == '\"' || c == '\'') {
						if (buffer != "") {
							result += buffer;
							buffer = "";
						}
						mode = M_STRING;
						string_char = c;
					}
					else if (c == '$') {
						if (buffer != "") {
							result += buffer;
							buffer = "";
						}
						mode = M_SHELL_VARIABLE;
					}
					else if (c == '(') {
						if (buffer != "") {
							result += buffer;
							buffer = "";
						}
						mode = M_SUBCOMMAND;
						subcommand_depth += 1;
					}
					else {
						buffer += c;
					}
				}
				else if (mode == M_STRING) {
					// While in string mode
					if (escape_next) {
						escape_next = false;
						// TODO: Escape more?
						if (c == 'n') {
							buffer += '\n';
						}
						else if (c == 'r') {
							buffer += '\r';
						}
						else if (c == 'e') {
							buffer += '\x1b';
						}
						else if (c == 't') {
							buffer += '\t';
						}
						else {
							buffer += c;
						}
					}
					else if (c == '`') {
						escape_next = true;
					}
					else if (c == string_char) {
						if (buffer != "") {
							result += buffer;
							buffer = "";
						}
						mode = M_RAW_STRING;
						string_char = 0;
					}
					else {
						buffer += c;
					}
				}
				else if (mode == M_SHELL_VARIABLE) {
					// Check if shell variable mode should be exited
					// For now, any variable name that doesn't contain
					// ", ' or ( is accepted
					// Those characters can be escaped
					if (escape_next) {
						escape_next = false;
						// TODO: Escape more?
						buffer += c;
					}
					else if (c == '`') {
						escape_next = true;
					}
					else if (c == '\"' || c == '\'') {
						string_char = c;
						mode = M_STRING;
						auto var = getenv(buffer.c_str());
						if (var != nullptr) {
							result += std::string(var);
						}
						buffer = "";
					}
					else if (c == '(') {
						mode = M_SUBCOMMAND;
						subcommand_depth += 1;
						auto var = getenv(buffer.c_str());
						if (var != nullptr) {
							result += std::string(var);
						}
						buffer = "";
					}
					else {
						buffer += c;
					}
				}
				else if (mode == M_SUBCOMMAND) {
					buffer += c;
					if (escape_next) {
						escape_next = false;
					}
					else if (c == '`') {
						escape_next = true;
					}
					else if (c == '(') {
						subcommand_depth += 1;
					}
					else if (c == ')') {
						subcommand_depth -= 1;
						if (subcommand_depth == 0) {
							// Exit subcommand mode
							buffer.erase(buffer.end() - 1);
							result += run_subcommand(buffer);
							buffer = "";
							mode = M_RAW_STRING;
						}
					}
				}
			}

			if (buffer != "") {
				if (mode == M_RAW_STRING) {
					result += buffer;
				}
				else if (mode == M_STRING) {
					// String wasn't ended, syntax error
					// "abc
					// TODO: Report error
				}
				else if (mode == M_SHELL_VARIABLE) {
					auto var = getenv(buffer.c_str());
					if (var != nullptr) {
						result += std::string(var);
					}
				}
				else if (mode == M_SUBCOMMAND) {
					if (subcommand_depth != 0) {
						// TODO: Throw error; subcommand incomplete
					}
					else {
						// Actually impossible, since if subcommand is ended
						// properly then it will exit M_SUBCOMMAND mode
						result += run_subcommand(buffer);
					}
				}
			}

			return result;
		}

		uint8_t process_command(std::vector<std::string>& args) {
			std::transform(
				args.begin(),
				args.end(),
				args.begin(),
				[](std::string s) {
					s = process_argument(s);
					trim::trim(s);
					return s;
				}
			);

			const auto& executable = args[0];

			auto who_is_running = env_is_true("who_is_running");

			if (executable == "exit") {
				if (who_is_running) {
					std::cerr << "$who_is_running: Internal command: exit" << std::endl;
				}
				return exit_command(args);
			}
			else if (executable == "cat") {
				if (who_is_running) {
					std::cerr << "$who_is_running: Internal command: cat" << std::endl;
				}
				return cat::cat(args);
			}
			else {
				if (who_is_running) {
					std::cerr << "$who_is_running: External command" << std::endl;
				}
				return external_command(args);
			}
		}

		uint8_t process_redirect(const std::string& line) {
			auto parts = split::split_command(line, " ");
			std::vector<std::string> non_redirect;
			autoundup2 auto_stdin(STDIN_FD);
			autoundup2 auto_stdout(STDOUT_FD);
			autoundup2 auto_stderr(STDERR_FD);
			for (auto part_iter = parts.begin(); part_iter != parts.end(); ++part_iter) {
				auto& part = *part_iter;
				trim::trim(part);

				// Check for unescaped redirection
				// Valid redirections:
				// <file.txt -OR- < file.txt
				// 0<file.txt -OR- 0< file.txt
				// >file.txt -OR- > file.txt
				// 1>file.txt -OR- 1> file.txt
				// 2>file.txt -OR- 2> file.txt
				// 2>&1 -OR- 2> &1 -OR- 1>&2 -OR- 1> &2 
				// >| or >> can be instead of |

				bool is_redirection = false;

				if (part[0] == '<' || part.size() > 1 && part[1] == '<' && part[0] != '`') {
					if (part[0] != '<' && part[0] != '0') {
						throw command_exception("kennsh: Syntax error: only fd 0 is supported for input redirection");
					}
					std::string path;
					if (part[0] == '<') {
						path = part.substr(1);
					}
					else {
						path = part.substr(2);
					}
					if (path == "") {
						// Look in next argument
						++part_iter;
						if (part_iter == parts.end()) {
							throw command_exception("kennsh: Redirect error: expected filename after <; found end of command");
						}
						path = *part_iter;
						trim::trim(path);
					}
					autoclose file(ewrap(open(path.c_str(), O_RDONLY)));
					ewrap(dup2(file, STDIN_FD));
					is_redirection = true;
				}
				if (is_redirection) {
					continue;
				}
				if (part[0] == '>' || part.size() > 1 && part[1] == '>' && part[0] != '`') {
					int fd = STDOUT_FD;
					const int M_CREATE = 0;
					const int M_APPEND = 1;
					const int M_OVERWRITE = 2;
					int mode = M_CREATE;
					if (part[0] != '>' && part[0] != '1' && part[0] != '2') {
						throw command_exception("kennsh: Syntax error: only fd 1 or 2 are supported for output redirection");
					}
					else if (part[0] != '>') {
						fd = part[0] - '0';
					}
					std::string path;
					if (part[0] == '>') {
						path = part.substr(1);
					}
					else {
						path = part.substr(2);
					}
					if (path != "" && (path[0] == '>' || path[0] == '|')) {
						if (path[0] == '>') {
							mode = M_APPEND;
						}
						else if (path[0] == '|') {
							mode = M_OVERWRITE;
						}
						path = path.substr(1);
					}
					if (path == "") {
						// Look in next argument
						++part_iter;
						if (part_iter == parts.end()) {
							throw command_exception("kennsh: Redirect error: expected filename after >; found end of command");
						}
						path = *part_iter;
						trim::trim(path);
					}
					int redirect_fd = -1;
					if (path[0] == '&') {
						path = path.substr(1);
						std::stringstream ss(path);
						ss >> redirect_fd;
						if (redirect_fd != 1 && redirect_fd != 2) {
							throw command_exception("kennsh: Syntax error: only fd 1 or 2 are supported for output redirection");
						}
					}
					if (redirect_fd != -1) {
						ewrap(dup2(redirect_fd, fd));
					}
					else {
						int flags = O_WRONLY;
						if (mode == M_CREATE) {
							if (access(path.c_str(), F_OK) == 0) {
								// File exists, error
								throw command_exception("kennsh: File already exists; use >> to append or >| to overwrite");
							}
							flags |= O_CREAT;
						}
						else if (mode == M_APPEND) {
							flags |= O_APPEND | O_CREAT;
						}
						else if (mode == M_OVERWRITE) {
							flags |= O_TRUNC;
						}
						autoclose file(ewrap(open(path.c_str(), flags, 0777)));
						ewrap(dup2(file, fd));
					}
					is_redirection = true;
				}

				if (!is_redirection) {
					non_redirect.push_back(part);
				}
			}
			auto result = process_command(non_redirect);
			return result;
		}

		uint8_t process_pipe(const std::string& line) {
			auto parts = split::split_command(line, "|", ">");
			if (parts.size() == 1) {
				// Skip the pipe pipeline
				return process_redirect(parts[0]);
			}

			// Setup pipe pipeline
			int old_pipe_read = STDIN_FD;
			int part_idx = 0;
			for (auto& part : parts) {
				trim::trim(part);

				int new_pipe[2];
				ewrap(pipe(new_pipe));
				int& new_pipe_read = new_pipe[0];
				int& new_pipe_write = new_pipe[1];

				pid_t child_pid = ewrap(fork());
				if (child_pid == 0) {
					// Child
					ewrap(close(new_pipe_read));
					ewrap(dup2(old_pipe_read, STDIN_FD));
					if (part_idx != parts.size() - 1) {
						ewrap(dup2(new_pipe_write, STDOUT_FD));
					}
					else {
						ewrap(close(new_pipe_write));
					}
					auto result = process_redirect(part);
					exit(result);
				}
				else {
					// Parent
					ewrap(close(new_pipe_write));
					old_pipe_read = new_pipe_read;
					if (part_idx == parts.size() - 1) {
						ewrap(close(old_pipe_read));
						int status = 0;
						ewrap(waitpid(child_pid, &status, 0));
						return WEXITSTATUS(status);
					}
				}

				part_idx += 1;
			}

			return 1;
		}
	}
}

uint8_t kennsh::command::handle(const std::string& line) {
	// Step 1: split for pipe
	// Step 2: find redirect
	// Step 3: execute command

	// TODO: handle error
	auto result = process_pipe(line);
	return result;
}
