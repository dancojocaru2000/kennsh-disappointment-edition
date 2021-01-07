#ifndef KENNSH_FILE_STUFF
#define KENNSH_FILE_STUFF

namespace kennsh {
	extern int STDIN_FD;
	extern int STDOUT_FD;
	extern int STDERR_FD;
	void set_nonblocking(int fd, bool nonblocking);
	struct autoclose {
		int fd;
		autoclose(int fd) : fd(fd) {}
		autoclose(autoclose const &) = delete;
		autoclose(autoclose&&) = default;
		~autoclose();
		autoclose& operator=(autoclose const &x) = delete;
		autoclose& operator=(autoclose&&) = default;
		void close();
		void cancel();
		operator int() const;
	};
	struct autoundup2 {
		int original;
		autoclose copy;
		autoundup2(int tosave);
		autoundup2(int oldfd, int newfd);
		autoundup2(autoundup2 const &) = delete;
		autoundup2(autoundup2&&) = default;
		~autoundup2();
		autoundup2& operator=(autoundup2 const &) = delete;
		autoundup2& operator=(autoundup2&&) = default;
		void undup2();
		void cancel();
	};
}

#endif