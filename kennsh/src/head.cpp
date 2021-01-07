#include "head.h"

// std::cout, std::endl
#include <iostream>
// std::remove_if, std::transport
#include <algorithm>
// stringstream => convert to number
#include <sstream>

extern "C" {
	// open
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	// dup
	#include <unistd.h>
}

#include "command.h"
#include "trim.h"
#include "file_stuff.h"
#include "c_error.h"
#include "file_iterator.h"
#include "iterators.hpp"

namespace kennsh::command::head {
	const int M_AUTO = 0;
	const int M_QUIET = 1;
	const int M_VERBOSE = 2;
	const int CM_LINES = 0;
	const int CM_BYTES = 1;

	template<class It>
	void print_bytes(It iter) {
		for (auto el : iter) {
			std::cout << (char)el;
		}
	}

	template<class It>
	void print_lines(It iter) {
		for (auto el : iter) {
			std::cout << (std::string)el << std::endl;
		}
	}

	void head_print(const std::string& file, int mode, int count_mode, long count) {
		int fd;
		if (file == "-") {
			fd = ewrap(dup(STDIN_FD));
		}
		else {
			fd = ewrap(open(file.c_str(), O_RDONLY));
		}
		autoclose _fd(fd);

		if (mode == M_VERBOSE) {
			if (file == "-") {
				std::cout << "==> standard input <==" << std::endl;
			}
			else {
				std::cout << "==> " << file << " <==" << std::endl;
			}
		}

		if (count_mode == CM_BYTES) {
			auto iter = file_iterator::bytes_iterator(fd);
			if (count >= 0) {
				print_bytes(iterators::take_iterator<uint8_t, file_iterator::bytes_iterator>(iter.begin(), iter.end(), count));
			}
			else {
				count = -count;
				print_bytes(iterators::skip_last_iterator<uint8_t, file_iterator::bytes_iterator>(iter.begin(), iter.end(), count));
				// print skiplastiterator
			}
		}
		else if (count_mode == CM_LINES) {
			auto iter = file_iterator::lines_iterator(fd);
			if (count >= 0) {
				print_lines(iterators::take_iterator<std::string, file_iterator::lines_iterator>(iter.begin(), iter.end(), count));
			}
			else {
				count = -count;
				print_lines(iterators::skip_last_iterator<std::string, file_iterator::lines_iterator>(iter.begin(), iter.end(), count));
				// print skiplastiterator
			}
		}

		if (mode == M_VERBOSE) {
			std::cout << std::endl;
		}
	}

	uint8_t head(std::vector<std::string> args) {
		args.erase(args.begin());
		int mode = M_AUTO;

		long count = 10;
		int count_mode = CM_LINES;

		std::vector<std::string> files;

		for (auto arg_iter = args.begin(); arg_iter != args.end(); arg_iter++) {
			auto& arg = *arg_iter;
			if (arg == "-") {
				files.push_back(arg);
			}
			else if (arg[0] == '-') {
				if (arg[1] == 'q') {
					mode = M_QUIET;
				}
				else if (arg[1] == 'v') {
					mode = M_VERBOSE;
				}
				else if (arg[1] == 'n') {
					count_mode = CM_LINES;
					arg_iter++;
					if (arg_iter == args.end()) {
						throw command_exception("head: Expected argument for -n");
					}
					std::stringstream ss(*arg_iter);
					ss >> count;
				}
				else if (arg[1] == 'c') {
					count_mode = CM_BYTES;
					arg_iter++;
					if (arg_iter == args.end()) {
						throw command_exception("head: Expected argument for -c");
					}
					std::stringstream ss(*arg_iter);
					ss >> count;
				}
				else {
					throw command_exception("head: Unknown command: " + arg);
				}
			}
			else {
				files.push_back(arg);
			}
		}

		std::transform(
			files.begin(),
			files.end(),
			files.begin(),
			[](std::string s) {
				trim::trim(s);
				return s;
			}
		);
		files.erase(
			std::remove_if(
				files.begin(),
				files.end(),
				[](std::string s) {
					return s == "";
				}
			),
			files.end()
		);

		if (files.size() == 0) {
			files.push_back("-");
		}

		if (mode == M_AUTO) {
			if (files.size() > 1) {
				mode = M_VERBOSE;
			}
			else {
				mode = M_QUIET;
			}
		}

		for (auto& file : files) {
			head_print(file, mode, count_mode, count);
		}
		std::cout << std::flush;

		// if (count_mode == CM_BYTES) {
		// 	std::cout << std::endl;
		// }

		return 0;
	}
}