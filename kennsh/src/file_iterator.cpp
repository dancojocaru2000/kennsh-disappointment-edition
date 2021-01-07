#include "file_iterator.h"

extern "C" {
	// read
	#include <unistd.h>
}

#include "c_error.h"

namespace kennsh::file_iterator {
	lines_iterator::lines_iterator() :
		eof(true),
		fd(-1) {}
	
	lines_iterator::lines_iterator(int fd) :
		eof(false),
		fd(fd),
		result_buffer(""),
		read_buffer_position(0) {
		this->read_buffer[4096] = 0;
		this->read_buffer[0] = 0;
		this->advance();
	}

	void lines_iterator::advance() {
		if (this->eof) {
			if (this->result_buffer != "") {
				this->result_buffer = "";
			}
			// Reached end
		}
		else {
			this->result_buffer = "";
			while (true) {
				if (this->read_buffer[this->read_buffer_position] == 0) {
					this->read_buffer_position = 0;
					auto bytes_read = ewrap(read(this->fd, this->read_buffer, 4096));
					if (bytes_read == 0) {
						this->eof = true;
						return;
					}
					else {
						this->read_buffer[bytes_read] = 0;
					}
				}
				while (this->read_buffer[this->read_buffer_position] != '\n' && this->read_buffer[this->read_buffer_position] != 0) {
					this->result_buffer += (char)this->read_buffer[this->read_buffer_position];
					this->read_buffer_position++;
				}
				if (this->read_buffer[this->read_buffer_position] == '\n') {
					this->read_buffer_position++;
					return;
				}
			}
		}
	}

	lines_iterator& lines_iterator::begin() {
		return *this;
	}

	lines_iterator lines_iterator::end() {
		return lines_iterator();
	}

	lines_iterator& lines_iterator::operator++() {
		this->advance();
		return *this;
	}

	std::string& lines_iterator::operator*() {
		return this->result_buffer;
	}

	void lines_iterator::operator++(int) {
		this->operator++();
	}

	bool lines_iterator::operator==(const lines_iterator& other) const {
		if (this->eof && other.eof && this->result_buffer == other.result_buffer) {
			return true;
		}
		return this->eof == other.eof && this->fd == other.fd && this->result_buffer == other.result_buffer;
	}

	bytes_iterator::bytes_iterator() :
		eof(true),
		fd(-1) {}
	
	bytes_iterator::bytes_iterator(int fd) :
		eof(false),
		fd(fd),
		read_buffer_position(0),
		read_buffer_size(0) {
		this->read_buffer[4096] = 0;
		this->advance();
	}

	void bytes_iterator::advance() {
		if (this->read_buffer_position < this->read_buffer_size) {
			this->read_buffer_position++;
		}
		else {
			if (eof) {
				// Nothing can be done
			}
			else {
				this->read_buffer_position = 0;
				auto bytes_read = ewrap(read(this->fd, this->read_buffer, 4096));
				this->read_buffer_size = bytes_read;
				this->read_buffer[bytes_read] = 0;
				if (bytes_read == 0) {
					this->eof = true;
				}
			}
		}
	}

	bytes_iterator& bytes_iterator::begin() {
		return *this;
	}

	bytes_iterator bytes_iterator::end() {
		return bytes_iterator();
	}

	bytes_iterator& bytes_iterator::operator++() {
		this->advance();
		return *this;
	}

	void bytes_iterator::operator++(int) {
		this->operator++();
	}

	bool bytes_iterator::operator==(const bytes_iterator& other) const {
		if (this->eof && other.eof) {
			return true;
		}
		return this->eof == other.eof && this->fd == other.fd;
	}

	uint8_t bytes_iterator::operator*() const {
		return this->read_buffer[this->read_buffer_position];
	}
}