#ifndef ALCHEMIST__COMMAND_HPP
#define ALCHEMIST__COMMAND_HPP

#include <string>
#include <cstdlib>

namespace alchemist {

typedef enum _client_command : uint8_t {
	WAIT = 0,
	// Connection
	HANDSHAKE = 1,
	REQUEST_ID = 2,
	CLIENT_INFO = 3,
	SEND_TEST_STRING = 4,
	REQUEST_TEST_STRING = 5,
	CLOSE_CONNECTION = 6,
	// Workers
	REQUEST_WORKERS = 11,
	YIELD_WORKERS = 12,
	SEND_ASSIGNED_WORKERS_INFO = 13,
	LIST_ALL_WORKERS = 14,
	LIST_ACTIVE_WORKERS = 15,
	LIST_INACTIVE_WORKERS = 16,
	LIST_ASSIGNED_WORKERS = 17,
	// Libraries
	LIST_AVAILABLE_LIBRARIES = 21,
	LOAD_LIBRARY = 22,
	UNLOAD_LIBRARY = 23,
	// Matrices
	SEND_MATRIX_INFO = 31,
	SEND_MATRIX_LAYOUT = 32,
	SEND_MATRIX_BLOCKS = 33,
	REQUEST_MATRIX_BLOCKS = 34,
	// Tasks
	RUN_TASK = 41,
	// Shutting down
	SHUTDOWN = 99
} client_command;

typedef enum _alchemist_command : uint8_t {
	_AM_IDLE = 0,
	_AM_START,
	_AM_SEND_INFO,
	_AM_PRINT_INFO,
	_AM_GROUP_OPEN_CONNECTIONS,
	_AM_GROUP_CLOSE_CONNECTIONS,
	_AM_FREE_GROUP,
	_AM_NEW_GROUP,
	_AM_NEW_SESSION,
	_AM_END_SESSION,
	_AM_NEW_MATRIX,
	_AM_CLIENT_MATRIX_LAYOUT,
	_AM_PRINT_DATA,
	_AM_WORKER_LOAD_LIBRARY,
	_AM_WORKER_RUN_TASK
} alchemist_command;

typedef enum _alchemist_error_code : uint8_t {
	ERR_NONE = 0,
	ERR_INVALID_HANDSHAKE,
	ERR_INVALID_CLIENT_ID,
	ERR_INVALID_SESSION_ID,
	ERR_INCONSISTENT_DATATYPES,
	ERR_NO_WORKERS,
	ERR_NONPOS_WORKER_REQUEST
} alchemist_error_code;

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
		case CLOSE_CONNECTION:
			return "CLOSE CONNECTION";
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
		case LIST_AVAILABLE_LIBRARIES:
			return "LIST AVAILABLE LIBRARIES";
		case UNLOAD_LIBRARY:
			return "UNLOAD LIBRARY";
		case SEND_MATRIX_INFO:
			return "SEND MATRIX INFO";
		case SEND_MATRIX_LAYOUT:
			return "SEND MATRIX LAYOUT";
		case SEND_MATRIX_BLOCKS:
			return "SEND MATRIX BLOCKS";
		case REQUEST_MATRIX_BLOCKS:
			return "REQUEST MATRIX BLOCKS";
		case RUN_TASK:
			return "RUN TASK";
		case SHUTDOWN:
			return "SHUTDOWN";
		default:
			return "INVALID COMMAND";
		}
}

inline const std::string get_command_name(const alchemist_command & c)
{
	switch (c) {
		case _AM_START:
			return "START";
		case _AM_SEND_INFO:
			return "SEND INFO";
		case _AM_PRINT_INFO:
			return "PRINT INFO";
		case _AM_GROUP_OPEN_CONNECTIONS:
			return "GROUP OPEN CONNECTIONS";
		case _AM_GROUP_CLOSE_CONNECTIONS:
			return "GROUP CLOSE CONNECTIONS";
		case _AM_NEW_GROUP:
			return "NEW GROUP";
		case _AM_FREE_GROUP:
			return "FREE GROUP";
		case _AM_NEW_SESSION:
			return "NEW SESSION";
		case _AM_END_SESSION:
			return "END SESSION";
		case _AM_NEW_MATRIX:
			return "NEW MATRIX";
		case _AM_CLIENT_MATRIX_LAYOUT:
			return "CLIENT MATRIX LAYOUT";
		case _AM_WORKER_LOAD_LIBRARY:
			return "WORKER LOAD LIBRARY";
		case _AM_WORKER_RUN_TASK:
			return "WORKER RUN TASK";
		case _AM_PRINT_DATA:
			return "PRINT DATA";
		default:
			return "INVALID COMMAND";
		}
}

inline const std::string get_error_name(const alchemist_error_code & ec)
{
	switch (ec) {
		case ERR_NONE:
			return "NONE";
		case ERR_INVALID_HANDSHAKE:
			return "INVALID HANDSHAKE";
		case ERR_INVALID_CLIENT_ID:
			return "INVALID CLIENT ID";
		case ERR_INVALID_SESSION_ID:
			return "INVALID SESSION ID";
		case ERR_INCONSISTENT_DATATYPES:
			return "INCONSISTENT DATATYPES";
		case ERR_NO_WORKERS:
			return "ERR NO WORKERS";
		case ERR_NONPOS_WORKER_REQUEST:
			return "ERR NONPOSITIVE WORKER REQUEST";
		default:
			return "INVALID COMMAND";
		}
}

}			// namespace alchemist

#endif
