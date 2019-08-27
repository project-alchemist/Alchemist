#include "WorkerSession.hpp"

namespace alchemist {

// =============================================================================================
//                                         WorkerSession
// =============================================================================================

WorkerSession::WorkerSession(tcp::socket _socket, GroupWorker & _group_worker) :
		Session(std::move(_socket)), group_worker(_group_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, GroupWorker & _group_worker, SessionID ID, ClientID _clientID) :
		Session(std::move(_socket), ID, _clientID), group_worker(_group_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, GroupWorker & _group_worker, SessionID ID, ClientID _clientID, Log_ptr & _log) :
		Session(std::move(_socket), ID, _clientID, _log), group_worker(_group_worker) { }

void WorkerSession::start()
{
	log->info("{} Connection established", preamble());
//	worker.write_session(shared_from_this());
	read_header();
}

void WorkerSession::remove_session()
{
	log->info("{} Removing session", preamble());
//	worker.remove_session();
}

int WorkerSession::handle_message()
{
//	log->info("{}", read_msg.to_string());
//	log->info("IN: {}", read_msg.to_string());

	client_command command = read_msg.cc;

	if (command == HANDSHAKE) {
		handle_handshake();
		read_header();
	}
	else {
		ClientID _clientID = read_msg.clientID;
		SessionID _sessionID = read_msg.sessionID;

		if (sessionID != _sessionID) {
			log->info("{} Error in WorkerSession: Wrong session ID", preamble());
		}

		switch (command) {
			case REQUEST_TEST_STRING:
				send_test_string();
				break;
			case SEND_TEST_STRING:
				send_response_string();
				break;
			case SEND_INDEXED_ROWS:
				handle_send_indexed_rows();
				break;
			case REQUEST_INDEXED_ROWS:
				handle_request_indexed_rows();
				break;
			case SEND_MATRIX_BLOCKS:
				receive_matrix_blocks();
				break;
			case REQUEST_MATRIX_BLOCKS:
				send_matrix_blocks();
				break;
		}

		read_header();
	}

	return 0;
}

void WorkerSession::handle_send_indexed_rows()
{
	MatrixID matrixID = read_msg.read_MatrixID();
	uint64_t row = 0, num_cols = 0, num_read_rows = 0;
	double value = 0.0;

	while (!read_msg.eom()) {
		try {
			read_msg.check_datatype(INDEXED_ROW);

			row = read_msg.get_uint64();
			num_cols = read_msg.get_uint64();

			log->info("{} Receiving row {} for matrix {}", preamble(), row, matrixID);

			for (uint64_t i = 0; i < num_cols; i++) {
				read_msg.get_double(value);
				group_worker.set_value(matrixID, row, i, value);
			}

			num_read_rows++;
		}
		catch (const std::exception& e) {
			log->info("{}", e.what());
		}
	}

	group_worker.print_data(matrixID);

	write_msg.start(clientID, sessionID, SEND_INDEXED_ROWS);
	write_msg.write_MatrixID(matrixID);
	write_msg.write_uint64(num_read_rows);

	flush();
}

void WorkerSession::handle_request_indexed_rows()
{
	MatrixID matrixID = read_msg.read_MatrixID();
	uint64_t row = 0, num_cols = 0, num_read_rows = 0;
	double value = 0.0;

	write_msg.start(clientID, sessionID, REQUEST_INDEXED_ROWS);
	write_msg.write_MatrixID(matrixID);

	while (!read_msg.eom()) {
		row = read_msg.read_uint64();
		num_cols = group_worker.get_num_local_cols(matrixID);

		write_msg.write_IndexedRow(row, num_cols);

		for (uint64_t i = 0; i < num_cols; i++) {
			group_worker.get_value(matrixID, row, i, value);
			write_msg.put_double(value);
		}
	}

	flush();
}

bool WorkerSession::send_matrix_blocks()
{
	uint32_t num_blocks = 0;
	uint64_t i, j;
	double temp;
	MatrixBlock_ptr in_block, out_block;

	MatrixID matrixID = read_msg.read_MatrixID();

	log->info("{} Sending data blocks for matrix {}", preamble(), matrixID);

	clock_t start = clock();

	write_msg.start(clientID, sessionID, REQUEST_MATRIX_BLOCKS);
	write_msg.write_MatrixID(matrixID);

	while (!read_msg.eom()) {

		MatrixBlock_ptr in_block = read_msg.read_MatrixBlock();

		out_block = std::make_shared<MatrixBlock<double>>(in_block->rows, in_block->cols);

		write_msg.write_MatrixBlock(out_block);

		for (i = out_block->rows[0]; i <= out_block->rows[1]; i += out_block->rows[2])
			for (j = out_block->cols[0]; j <= out_block->cols[1]; j += out_block->cols[2]) {
				group_worker.get_value(matrixID, i, j, temp);
				out_block->write_next(&temp);
			}

		num_blocks++;
	}

	clock_t end = clock();
	log->info("{} Sending data blocks took {}ms", preamble(), 1000.0*((double) (end - start))/((double) CLOCKS_PER_SEC));
	flush();

	return true;
}

bool WorkerSession::receive_matrix_blocks()
{
	uint32_t num_blocks = 0;
	uint64_t i, j;
	double temp;

	MatrixID matrixID = read_msg.read_MatrixID();

	log->info("{} Receiving data blocks for matrix {}", preamble(), matrixID);

	clock_t start = clock();

	while (!read_msg.eom()) {

		MatrixBlock_ptr block = read_msg.read_MatrixBlock();

		for (i = block->rows[0]; i <= block->rows[1]; i += block->rows[2])
			for (j = block->cols[0]; j <= block->cols[1]; j += block->cols[2]) {
				block->read_next(&temp);
				group_worker.set_value(matrixID, i, j, temp);
			}

		num_blocks++;
//		log->info("{} Matrix {}: Received matrix block (rows {}-{}, columns {}-{})", preamble(), matrixID, row_start, row_end, col_start, col_end);
	}

	write_msg.start(clientID, sessionID, SEND_MATRIX_BLOCKS);
	write_msg.write_uint16(matrixID);
	write_msg.write_uint32(num_blocks);

	clock_t end = clock();
	log->info("{} Receiving data blocks took {}ms", preamble(), 1000.0*((double) (end - start))/((double) CLOCKS_PER_SEC));
	flush();

	return true;
}

bool WorkerSession::send_test_string()
{
//	char buffer[4];
//	std::stringstream test_str;
//
//	sprintf(buffer, "%03d", worker.getID());
//	test_str << "This is a test string from Alchemist worker " << buffer;
//
//	write_msg.start(clientID, ID, REQUEST_TEST_STRING);
//	write_msg.write_string(test_str.str());
//	flush();

	return true;
}


//int WorkerSession::receive_test_string(const char * data, const uint32_t length)
//{
//	log->info("{} Test string: {}", preamble(), string(data, length));
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

	write_msg.start(clientID, sessionID, SEND_TEST_STRING);
	write_msg.write_string(test_str);
	flush();

	return true;
}


}			// namespace alchemist
