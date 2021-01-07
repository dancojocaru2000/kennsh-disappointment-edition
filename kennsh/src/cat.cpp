#include "cat.h"

#include <iostream>
// std::setw
#include <iomanip>

extern "C" {
	// dup
	#include <unistd.h>
	// open
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
}

#include "c_error.h"
#include "command.h"
#include "file_iterator.h"
#include "trim.h"
#include "file_stuff.h"

namespace kennsh::command::cat {
	void cat_print(const std::string& path, bool b, bool E, bool n, bool s) {
		// Initialize file
		int fd;
		if (path == "-") {
			fd = ewrap(dup(STDIN_FD));
		}
		else {
			fd = ewrap(open(path.c_str(), O_RDONLY));
		}
		autoclose file(fd);

		// Start work
		int current_line = 0;
		bool previous_line_empty = false;

		kennsh::file_iterator::lines_iterator lines_it(file);

		for (auto line : lines_it) {
			trim::rtrim(line);
			auto chk = trim::ltrim_copy(line);
			auto is_line_empty = (chk == "");

			if (s) {
				if (is_line_empty) {
					if (!previous_line_empty) {
						previous_line_empty = true;
					}
					else {
						continue;
					}
				}
				else {
					previous_line_empty = false;
				}
			}

			if (n) {
				if (!b || !is_line_empty) {
					current_line += 1;
					std::cout << std::setw(6) << current_line << std::setw(0) << "  ";
				}
			}

			std::cout << line;

			if (E) {
				std::cout << '$';
			}

			std::cout << std::endl;
		}
	}

	uint8_t cat(const std::vector<std::string>& args) {
		bool b = false;
		bool E = false;
		bool n = false;
		bool s = false;

		for (auto item_iter = args.begin() + 1; item_iter != args.end(); ++item_iter) {
			auto& item = *item_iter;
			if (item[0] == '-') {
				for (auto c_iter = item.begin() + 1; c_iter != item.end(); c_iter++) {
					auto& c = *c_iter;
					if (c == 'b') {
						b = true;
						n = true;
					}
					else if (c == 'E') {
						E = true;
					}
					else if (c == 'n') {
						n = true;
					}
					else if (c == 's') {
						s = true;
					}
					else {
						throw command_exception("cat: Unrecognized option: -" + c);
					}
				}
			}
			else {
				cat_print(item, b, E, n, s);
			}
		}

		return 0;
	}
}