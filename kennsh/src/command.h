#ifndef KENNSH_COMMAND
#define KENNSH_COMMAND

#include <string>
#include <vector>
#include <exception>

namespace kennsh {
	class command_exception: public std::exception {
	public:
		std::string error_description;
		command_exception(std::string ed) : error_description(ed) {}

		virtual const char* what() const throw() {
			return error_description.c_str();
		}
	};

	class exit_request: public command_exception {
	public:
		uint8_t exit_code;
		bool use_last_status_code;
		exit_request(uint8_t code): 
			command_exception("kennsh: exit command ran"),
			exit_code(code),
			use_last_status_code(false) {}
		exit_request():
			command_exception("kennsh: exit command ran"),
			exit_code(0),
			use_last_status_code(true) {}
	};
	
	namespace command {
		uint8_t handle(const std::string& line);
		uint8_t process_command(std::vector<std::string>& args);
		std::string run_subcommand(const std::string& subcommand);
	}
}

#endif