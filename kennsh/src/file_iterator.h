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
		bool operator==(const lines_iterator& other) const;
		bool operator!=(const lines_iterator& other) const {
			return !this->operator==(other);
		}
	};
	class bytes_iterator {
	private:
		bool eof;
		int fd;
		char read_buffer[4097];
		int read_buffer_position;
		int read_buffer_size;

		void advance();
		bytes_iterator();
	public:
		bytes_iterator(int fd);
		bytes_iterator& begin();
		bytes_iterator end();
		bytes_iterator& operator++();
		uint8_t operator*() const;
		void operator++(int);
		bool operator==(const bytes_iterator& other) const;
		bool operator!=(const bytes_iterator& other) const {
			return !this->operator==(other);
		}
	};
}

#endif