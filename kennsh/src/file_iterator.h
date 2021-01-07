#ifndef KENNSH_FILE_ITERATOR
#define KENNSH_FILE_ITERATOR

#include <string>

namespace kennsh::file_iterator {
	class lines_iterator {
	private:
		bool eof;
		int fd;
		std::string result_buffer;
		char read_buffer[4097];
		int read_buffer_position;

		void advance();
		lines_iterator();
	public:
		lines_iterator(int fd);
		lines_iterator& begin();
		lines_iterator end();
		lines_iterator& operator++();
		std::string& operator*();
		void operator++(int);
		bool operator==(const lines_iterator& other);
		bool operator!=(const lines_iterator& other) {
			return !this->operator==(other);
		}
	};
}

#endif