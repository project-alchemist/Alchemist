#ifndef ALCHEMIST__COMMAND_HPP
#define ALCHEMIST__COMMAND_HPP

#include <string>
#include <cstdlib>

namespace alchemist {

typedef enum _client_command : uint8_t {
	WAIT = 0,
	HANDSHAKE,
	REQUEST_ID,
	SEND_TEST_STRING,
	REQUEST_TEST_STRING,
	REQUEST_WORKERS,
	YIELD_WORKERS,
	SEND_ASSIGNED_WORKERS_INFO,
	LIST_ALL_WORKERS,
	LIST_ACTIVE_WORKERS,
	LIST_INACTIVE_WORKERS,
	LIST_ASSIGNED_WORKERS,
	LOAD_LIBRARY,
	RUN_TASK,
	UNLOAD_LIBRARY,
	SHUT_DOWN
} client_command;

typedef enum _alchemist_command : uint8_t {
	IDLE                            = 0,
	START                           = 1,
	SEND_INFO                       = 2,
	ACCEPT_CONNECTION               = 3
} alchemist_command;

inline const std::string get_command_name(const client_command & c)
{
	switch (c) {
		case WAIT:
			return "WAIT";
		case HANDSHAKE:
			return "HANDSHAKE";
		case REQUEST_ID:
			return "REQUEST ID";
		case SEND_TEST_STRING:
			return "SEND TEST STRING";
		case REQUEST_TEST_STRING:
			return "REQUEST TEST STRING";
		case REQUEST_WORKERS:
			return "REQUEST WORKERS";
		case YIELD_WORKERS:
			return "YIELD WORKERS";
		case SEND_ASSIGNED_WORKERS_INFO:
			return "SEND ASSIGNED WORKERS INFO";
		case LIST_ALL_WORKERS:
			return "LIST ALL WORKERS";
		case LIST_ACTIVE_WORKERS:
			return "LIST ACTIVE WORKERS";
		case LIST_INACTIVE_WORKERS:
			return "LIST INACTIVE WORKERS";
		case LIST_ASSIGNED_WORKERS:
			return "LIST ASSIGNED WORKERS";
		case LOAD_LIBRARY:
			return "LOAD LIBRARY";
		case RUN_TASK:
			return "RUN TASK";
		case UNLOAD_LIBRARY:
			return "UNLOAD LIBRARY";
		default:
			return "INVALID COMMAND";
		}
}

inline const std::string get_command_name(const alchemist_command & c)
{
	switch (c) {
		case START:
			return "START";
		case SEND_INFO:
			return "SEND_INFO";
		case ACCEPT_CONNECTION:
			return "ACCEPT CONNECTION";
		default:
			return "INVALID COMMAND";
		}
}

}				// UNLOAD_LIBRARY

#endif
