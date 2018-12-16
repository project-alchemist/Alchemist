#include "WorkerSession.hpp"

namespace alchemist {

// =============================================================================================
//                                         WorkerSession
// =============================================================================================

WorkerSession::WorkerSession(tcp::socket _socket, GroupWorker & _group_worker) :
		Session(std::move(_socket)), group_worker(_group_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, GroupWorker & _group_worker, Session_ID _ID, Client_ID _client_ID) :
		Session(std::move(_socket), _ID, _client_ID), group_worker(_group_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, GroupWorker & _group_worker, Session_ID _ID, Client_ID _client_ID, Log_ptr & _log) :
		Session(std::move(_socket), _ID, _client_ID, _log), group_worker(_group_worker) { }

void WorkerSession::start()
{
	log->info("{} Connection established", session_preamble());
//	worker.add_session(shared_from_this());
	read_header();
}

void WorkerSession::remove_session()
{
	log->info("{} Removing session", session_preamble());
//	worker.remove_session();
}

int WorkerSession::handle_message()
{
//	log->info("{}", read_msg.to_string());


	client_command command = read_msg.cc;

	if (command == HANDSHAKE) {
		handle_handshake();
		read_header();
	}
	else {
		Client_ID _client_ID = read_msg.client_ID;
		Session_ID _session_ID = read_msg.session_ID;

		if (session_ID != _session_ID) {
			log->info("{} Error in WorkerSession: Wrong session ID", session_preamble());
		}

		switch (command) {
			case REQUEST_TEST_STRING:
				send_test_string();
				read_header();
				break;
			case SEND_TEST_STRING:
				send_response_string();
				read_header();
				break;
			case SEND_MATRIX_BLOCKS:
				receive_matrix_blocks();
				read_header();
				break;
			case REQUEST_MATRIX_BLOCKS:
				send_matrix_blocks();
				read_header();
				break;
		}
	}

	return 0;
}

bool WorkerSession::send_matrix_blocks()
{
	uint32_t num_blocks = 0;
	uint64_t row_start, row_end, col_start, col_end;
	double temp;

	Matrix_ID matrix_ID = read_msg.read_uint16();

	write_msg.start(client_ID, session_ID, REQUEST_MATRIX_BLOCKS);
	write_msg.add_uint16(matrix_ID);

	while (!read_msg.eom()) {

		row_start = read_msg.read_uint64();
		row_end   = read_msg.read_uint64();
		col_start = read_msg.read_uint64();
		col_end   = read_msg.read_uint64();

		write_msg.add_uint64(row_start);
		write_msg.add_uint64(row_end);
		write_msg.add_uint64(col_start);
		write_msg.add_uint64(col_end);

		for (auto i = row_start; i <= row_end; i++)
			for (auto j = col_start; j <= col_end; j++) {
				group_worker.get_value(matrix_ID, i, j, temp);
				write_msg.add_double(temp);
			}

		num_blocks++;
	}
	flush();

	return true;
}

bool WorkerSession::receive_matrix_blocks()
{
	uint32_t num_blocks = 0;
	uint64_t i, j, row_start, row_end, col_start, col_end;

	Matrix_ID matrix_ID = read_msg.read_uint16();

	while (!read_msg.eom()) {

		row_start = read_msg.read_uint64();
		row_end   = read_msg.read_uint64();
		col_start = read_msg.read_uint64();
		col_end   = read_msg.read_uint64();

		for (i = row_start; i <= row_end; i++)
			for (j = col_start; j <= col_end; j++)
				group_worker.set_value(matrix_ID, i, j, read_msg.read_double());

		num_blocks++;
		log->info("{} Matrix {}: Received matrix block (rows {}-{}, columns {}-{})", session_preamble(), matrix_ID, row_start, row_end, col_start, col_end);
	}

	write_msg.start(client_ID, session_ID, SEND_MATRIX_BLOCKS);
	write_msg.add_uint16(matrix_ID);
	write_msg.add_uint32(num_blocks);
	flush();

	return true;
}

bool WorkerSession::send_test_string()
{
//	char buffer[4];
//	std::stringstream test_str;
//
//	sprintf(buffer, "%03d", worker.get_ID());
//	test_str << "This is a test string from Alchemist worker " << buffer;
//
//	write_msg.start(client_ID, ID, REQUEST_TEST_STRING);
//	write_msg.add_string(test_str.str());
//	flush();

	return true;
}


//int WorkerSession::receive_test_string(const char * data, const uint32_t length)
//{
//	log->info("{} Test string: {}", session_preamble(), string(data, length));
//
//	return 0;
//}
//
//string WorkerSession::get_test_string()
//{
//	std::stringstream ss;
//	ss << "This is a test string from Alchemist worker " << ID;
//
//	return ss.str();
//}


bool WorkerSession::send_response_string()
{
	string test_str = "Alchemist worker received: '";
	test_str += read_msg.read_string();
	test_str += "'";

	write_msg.start(client_ID, session_ID, SEND_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
}


}			// namespace alchemist
