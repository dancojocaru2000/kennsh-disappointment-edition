#ifndef KENNSH_C_ERROR
#define KENNSH_C_ERROR

#include <exception>
#include <cstring>
extern "C" {
	#include <errno.h>
}

namespace kennsh {
	class c_error: public std::exception {
	public:
		int num;

		c_error() : num(errno) {}
		c_error(int errnum) : num(errnum) {}

		virtual const char* what() const throw() {
			return strerror(this->num);
		}
	};

	template <class T>
	T ewrap(T value) {
		if (value == -1) {
			throw c_error();
		}
		return value;
	}
}

#endif