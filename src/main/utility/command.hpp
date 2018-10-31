#ifndef ALCHEMIST__COMMAND_HPP
#define ALCHEMIST__COMMAND_HPP

#include <string>
#include <cstdlib>

namespace alchemist {

typedef enum _client_command : uint8_t {
	WAIT = 0,
	HANDSHAKE,
	REQUEST_ID,
	CLIENT_INFO,
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
	MATRIX_INFO,
	MATRIX_LAYOUT,
	SEND_MATRIX_BLOCKS,
	REQUEST_MATRIX_BLOCKS,
	SHUT_DOWN
} client_command;

typedef enum _alchemist_command : uint8_t {
	IDLE = 0,
	START,
	SEND_INFO,
	ACCEPT_CONNECTION,
	SAY_SOMETHING,
	NEW_SESSION,
	END_SESSION,
	NEW_MATRIX,
	CLIENT_MATRIX_LAYOUT,
	PRINT_DATA,
	WORKER_LOAD_LIBRARY,
	WORKER_RUN_TASK
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
		case CLIENT_INFO:
			return "CLIENT INFO";
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
		case MATRIX_INFO:
			return "MATRIX INFO";
		case MATRIX_LAYOUT:
			return "MATRIX LAYOUT";
		case SEND_MATRIX_BLOCKS:
			return "SEND MATRIX BLOCKS";
		case REQUEST_MATRIX_BLOCKS:
			return "REQUEST MATRIX BLOCKS";
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
		case SAY_SOMETHING:
			return "SAY SOMETHING";
		case NEW_SESSION:
			return "NEW SESSION";
		case END_SESSION:
			return "END SESSION";
		case NEW_MATRIX:
			return "NEW MATRIX";
		case CLIENT_MATRIX_LAYOUT:
			return "CLIENT MATRIX LAYOUT";
		case WORKER_LOAD_LIBRARY:
			return "WORKER LOAD LIBRARY";
		case WORKER_RUN_TASK:
			return "WORKER RUN TASK";
		case PRINT_DATA:
			return "PRINT DATA";
		default:
			return "INVALID COMMAND";
		}
}

}			// namespace alchemist

#endif
