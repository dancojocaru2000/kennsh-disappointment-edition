#include "file_stuff.h"

extern "C" {
	#include <unistd.h>
	#include <fcntl.h>
}

#include "c_error.h"

int kennsh::STDIN_FD = STDIN_FILENO;
int kennsh::STDOUT_FD = STDOUT_FILENO;
int kennsh::STDERR_FD = STDERR_FILENO;

void kennsh::set_nonblocking(int fd, bool nonblocking) {
	int previous = ewrap(fcntl(fd, F_GETFL));
	int current;
	if (nonblocking) {
		current = previous | O_NONBLOCK;
	}
	else {
		current = previous & (~O_NONBLOCK);
	}
	if (current != previous) {
		ewrap(fcntl(fd, F_SETFL, current));
	}
}

kennsh::autoclose::~autoclose() {
	try {
		this->close();
	}
	catch (...) {}
}

void kennsh::autoclose::close() {
	ewrap(::close(this->fd));
}

void kennsh::autoclose::cancel() {
	this->fd = -1;
}

kennsh::autoclose::operator int() const {
	return this->fd;
}

kennsh::autoundup2::autoundup2(int oldfd, int newfd) : autoundup2(newfd) {
	// this->original = newfd;
	// this->copy = std::move(kennsh::autoclose(ewrap(dup(newfd))));
	ewrap(dup2(oldfd, newfd));
}

kennsh::autoundup2::autoundup2(int tosave) : 
	original(tosave), 
	copy(ewrap(dup(tosave))) {
	// this->original = tosave;
	// this->copy = std::move(kennsh::autoclose(ewrap(dup(tosave))));
}

kennsh::autoundup2::~autoundup2() {
	try {
		this->undup2();
	}
	catch (...) {}
}

void kennsh::autoundup2::undup2() {
	if (this->copy >= 0 && this->original >= 0) {
		ewrap(dup2(this->copy, this->original));
	}
}

void kennsh::autoundup2::cancel() {
	this->copy.cancel();
	this->original = -1;
}
