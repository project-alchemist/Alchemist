#include "DriverSession.hpp"

namespace alchemist {

// =============================================================================================
//                                         DriverSession
// =============================================================================================

DriverSession::DriverSession(tcp::socket _socket, GroupDriver & _group_driver) :
		Session(std::move(_socket)), group_driver(_group_driver), num_group_workers(0) { }

DriverSession::DriverSession(tcp::socket _socket, GroupDriver & _group_driver, ClientID _clientID) :
		Session(std::move(_socket), 0, _clientID), group_driver(_group_driver), num_group_workers(0) { }

DriverSession::DriverSession(tcp::socket _socket, GroupDriver & _group_driver, ClientID _clientID, Log_ptr & _log) :
		Session(std::move(_socket), 0, _clientID, _log), group_driver(_group_driver), num_group_workers(0) { }

void DriverSession::start()
{
	log->info("DriverSession: START");
	log->info("{} Connection established", preamble());
//	driver.write_session(shared_from_this());
	read_header();
}

bool DriverSession::send_response_string()
{
	return true;
}

int DriverSession::handle_message()
{
//	log->info("Received message from Session {} at {}", getID(), get_address().c_str());
	log->info("IN: {}", read_msg.to_string());
//	log->info("{}", read_msg.cc);

	client_command command = read_msg.cc;

	if (command == HANDSHAKE) {
		handle_handshake();
		read_header();
	}
	else {
		if (clientID == read_msg.clientID && sessionID == read_msg.sessionID) {
			if (command == SHUTDOWN) handle_shutdown();
			else {
				switch (command) {
				case REQUEST_ID:
					handle_request_ID();
					break;
				case CLIENT_INFO:
					handle_client_info();
					break;
				case SEND_TEST_STRING:
					handle_send_test_string();
					break;
				case REQUEST_TEST_STRING:
					handle_request_test_string();
					break;
				case CLOSE_CONNECTION:
					handle_close_connection();
					break;
				// Workers
				case REQUEST_WORKERS:
					handle_request_workers();
					break;
				case YIELD_WORKERS:
					handle_yield_workers();
					break;
				case SEND_ASSIGNED_WORKERS_INFO:
					handle_send_assigned_worker_info();
					break;
				case LIST_ALL_WORKERS:
					handle_list_all_workers();
					break;
				case LIST_ACTIVE_WORKERS:
					handle_list_active_workers();
					break;
				case LIST_INACTIVE_WORKERS:
					handle_list_inactive_workers();
					break;
				case LIST_ASSIGNED_WORKERS:
					handle_list_assigned_workers();
					break;
				// Libraries
				case LIST_AVAILABLE_LIBRARIES:
					handle_list_available_libraries();
					break;
				case LOAD_LIBRARY:
					handle_load_library();
					break;
				case UNLOAD_LIBRARY:
					handle_unload_library();
					break;
					// Matrices
				case SEND_MATRIX_INFO:
					handle_matrix_info();
					break;
				case SEND_MATRIX_LAYOUT:
					handle_matrix_layout();
					break;
				case SEND_MATRIX_BLOCKS:
					handle_send_matrix_blocks();
					break;
				case REQUEST_MATRIX_BLOCKS:
					handle_request_matrix_blocks();
					break;
					// Tasks
				case RUN_TASK:
					handle_run_task();
					break;
				default:
					handle_invalid_command();
					break;
				}
				read_header();
			}
		}
		else log->error("{} Error in DriverSession: Wrong ID {}:{}", preamble(), read_msg.clientID, read_msg.sessionID);
	}

	return 0;
}

void DriverSession::handle_request_ID()
{

}

void DriverSession::handle_client_info()
{
//	ClientID _clientID = driver.allocate_clientID();
//	num_group_workers = read_msg.read_uint16();
	log_dir = read_msg.read_string();

	string response_str = "Logs saved to: ";
	response_str += log_dir;

	write_msg.start(clientID, sessionID, CLIENT_INFO);
	write_msg.write_string(response_str);
	flush();

	log->info(response_str);
}

void DriverSession::handle_send_test_string()
{
	string test_str = "The Alchemist driver for session ";
	test_str += std::to_string(sessionID);
	test_str += " received: '";
	test_str += read_msg.read_string();
	test_str += "'.";

	// Send response
	write_msg.start(clientID, sessionID, SEND_TEST_STRING);
	write_msg.write_string(test_str);
	flush();
}

void DriverSession::handle_request_test_string()
{
	string test_str = "This is a test string from the Alchemist driver for session ";
	test_str += std::to_string(sessionID);
	test_str += ".";

	write_msg.start(clientID, sessionID, REQUEST_TEST_STRING);
	write_msg.write_string(test_str);
	flush();
}

void DriverSession::handle_close_connection()
{

}

void DriverSession::handle_request_workers()
{
	uint16_t num_requested_workers = read_msg.read_uint16();

	write_msg.start(clientID, sessionID, REQUEST_WORKERS);

	if (num_requested_workers > 0) {
		map<WorkerID, WorkerInfo> allocated_workers = group_driver.allocate_workers(num_requested_workers);

		if (allocated_workers.size() > 0) {
			uint16_t num_allocated_workers = allocated_workers.size();
			write_msg.write_uint16(num_allocated_workers);
			for (auto it = allocated_workers.begin(); it != allocated_workers.end(); it++) {
				write_msg.write_uint16(it->first);
				write_msg.write_string(it->second.hostname);
				write_msg.write_string(it->second.address);
				write_msg.write_uint16(it->second.port);
			}
		}
		else {
			write_msg.write_uint16(0);
			write_msg.write_string(string("ERROR: No Alchemist workers currently available, try again later."));
		}
	}
	else {
		write_msg.write_uint16(0);
		write_msg.write_string(string("ERROR: Number of requested workers must be positive."));
	}

	flush();
}

void DriverSession::handle_yield_workers()
{
	vector<WorkerID> yielded_workers;
	WorkerID workerID;
	map<WorkerID, WorkerInfo>::iterator it;

	write_msg.start(clientID, sessionID, YIELD_WORKERS);

	if (read_msg.eom()) {
		for (auto it = workers.begin(); it != workers.end(); it++)
			yielded_workers.push_back(it->first);
	}
	else {
		while (!read_msg.eom()) {
			workerID = read_msg.read_uint16();
//			for (auto it = group_driver.workers.begin(); it != group_driver.workers.end(); it++)
//				log->info("YYYYYYYYYYYYYY {}", it->first);
			it = group_driver.workers.find(workerID);
			if (it != workers.end()) {
				yielded_workers.push_back(workerID);
			}
		}
	}

	if (yielded_workers.size() != 0) {
		vector<WorkerID> deallocated_workers = group_driver.deallocate_workers(yielded_workers);
		for (auto it = deallocated_workers.begin(); it < deallocated_workers.end(); it++)
			write_msg.write_uint16(*it);
	}

	flush();



//
//	uint16_t num_requested_workers = read_msg.read_uint16();
//
//	deallocate_workers();
//	write_msg.start(clientID, sessionID, YIELD_WORKERS);
//	write_msg.write_string(string("Alchemist workers have been deallocated"));
//	flush();
//		write_msg.start(clientID, sessionID, YIELD_WORKERS);
//
//		uint16_t num_requested_workers = read_msg.read_uint16();
//
//		if (num_requested_workers > 0) {
//
//			map<WorkerID, WorkerInfo> allocated_group = group_driver.allocate_workers(num_requested_workers);
//
//			if (allocated_group.size() > 0) {
//				uint16_t group_size = allocated_group.size();
//				write_msg.write_uint16(group_size);
//				for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
//					write_msg.write_uint16(it->first);
//					write_msg.write_string(it->second.hostname);
//					write_msg.write_string(it->second.address);
//					write_msg.write_uint16(it->second.port);
//				}
//			}
//			else {
//				write_msg.write_uint16(0);
//				write_msg.write_string(string("ERROR: Insufficient number of Alchemist workers available"));
//				write_msg.write_string(string("       Try again later or request fewer workers"));
//			}
//
//
//	//		std::stringstream list_of_alchemist_workers;
//	//		char buffer[4];
//	//
//	//		list_of_alchemist_workers << session_preamble();
//	//
//	//		if (num_workers == 1)
//	//			list_of_alchemist_workers << "Allocating 1 worker:", getID(), get_address().c_str();
//	//		else
//	//			list_of_alchemist_workers << "Allocating " << num_workers << " workers:";
//	//		list_of_alchemist_workers << std::endl;
//
//	//		auto workers = driver.allocate_workers(clientID, num_workers);
//
//	//		write_uint16(num_workers);
//	//		const int groupIDs[num_workers+1];
//
//	//		groupIDs[0] = 0;
//	//
//	//		for (auto it = workers.begin(); it != workers.end(); it++) {
//	//			sprintf(buffer, "%03d", it->first);
//	//			list_of_alchemist_workers << SPACE;
//	//			list_of_alchemist_workers << "    Worker-" << string(buffer) << " running on " << it->second.hostname << " ";
//	//			list_of_alchemist_workers << it->second.address << ":" << it->second.port << std::endl;
//	//
//	////			groupIDs[i+1] = peerID;
//	//
//	//			write_uint16(it->first);
//	//			write_string(it->second.hostname);
//	//			write_string(it->second.address);
//	//			write_uint16(it->second.port);
//	//		}
//
//	//			auto layout_rr = driver.prepare_data_layout_table(num_workers, num_group_workers);
//
//	//			write_msg.write_uint16(num_group_workers);
//	//			for (int i = 0; i < num_group_workers; i++) {
//	//				for (int j = 0; j < num_workers; j++) {
//	//					write_msg.write_float(layout_rr[i][j][0]);
//	//					write_msg.write_float(layout_rr[i][j][1]);
//	//				}
//	//			}
//
//	//		log->info(list_of_alchemist_workers.str());
//
//	//
//	//		MPI_Status status;
//	//
//	//		MPI_Recv(&sessionID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//	//
//	//		uint16_t num_peers = 0;
//	//
//	//		MPI_Recv(&num_peers, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//	//
//	//		uint16_t peerID;
//	//		const int groupIDs[num_peers+1];
//	//		const int group_peerIDs[num_peers];
//	//
//	//		groupIDs[0] = 0;			// First member is the driver
//	//		for (uint16_t i = 0; i < num_peers; i++) {
//	//			MPI_Recv(&peerID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//	//			groupIDs[i+1] = peerID;
//	//			group_peerIDs[i] = peerID;
//	//		}
//	//
//	//		MPI_Group world_group;
//	//		MPI_Group temp_group;
//	//
//	//		MPI_Comm_group(world, &world_group);
//	//
//	//		MPI_Group_incl(world_group, (int) (num_peers+1), groupIDs, &temp_group);
//	//		MPI_Comm_create_group(world, temp_group, 0, &group);			// Create a new communicator based on the groupIDs
//	//
//
//
//
//
//		}
//		else {
//			write_msg.write_uint16(0);
//			write_msg.write_string(string("ERROR: Number of requested workers must be positive"));
//		}



}

void DriverSession::handle_send_assigned_worker_info()
{
//	send_assigned_worker_info();
}

void DriverSession::handle_list_all_workers()
{
	log->info("{} Sending list of all workers", preamble());

	string pre = read_msg.read_string();

	write_msg.start(clientID, sessionID, LIST_ALL_WORKERS);
	write_msg.write_string(group_driver.list_all_workers(pre));
	flush();
}

void DriverSession::handle_list_active_workers()
{
	log->info("{} Sending list of active workers", preamble());

	string pre = read_msg.read_string();

	write_msg.start(clientID, sessionID, LIST_ACTIVE_WORKERS);
//	write_msg.write_uint16(group_driver.get_num_workers());
	write_msg.write_string(group_driver.list_active_workers(pre));
	flush();
}

void DriverSession::handle_list_inactive_workers()
{
	log->info("{} Sending list of inactive workers", preamble());

	string pre = read_msg.read_string();

	write_msg.start(clientID, sessionID, LIST_INACTIVE_WORKERS);
//	write_msg.write_uint16(group_driver.get_num_workers());
	write_msg.write_string(group_driver.list_inactive_workers(pre));
	flush();
}

void DriverSession::handle_list_assigned_workers()
{
	log->info("{} Sending list of assigned workers", preamble());

	string pre = read_msg.read_string();

	write_msg.start(clientID, sessionID, LIST_ASSIGNED_WORKERS);
//	write_msg.write_uint16(group_driver.get_num_workers());
	write_msg.write_string(group_driver.list_allocated_workers(pre));
	flush();

}

void DriverSession::handle_list_available_libraries()
{
//	list_available_libraries();
}

void DriverSession::handle_load_library()
{
	string lib_name = read_msg.read_string();
	string lib_path = read_msg.read_string();

	LibraryID libID = group_driver.load_library(lib_name, lib_path);

	write_msg.start(clientID, sessionID, LOAD_LIBRARY);
	write_msg.write_LibraryID(libID);
	flush();
}

void DriverSession::handle_unload_library()
{
//	unload_library();
}

void DriverSession::handle_matrix_info()
{
	ArrayInfo_ptr x = read_msg.read_ArrayInfo();

	send_matrix_info(group_driver.new_matrix(x));
}

void DriverSession::handle_matrix_layout()
{

}

void DriverSession::handle_send_matrix_blocks()
{

}

void DriverSession::handle_request_matrix_blocks()
{

}

void DriverSession::handle_run_task()
{
	write_msg.start(clientID, sessionID, RUN_TASK);

	group_driver.run_task(read_msg, write_msg);

	flush();
}

void DriverSession::handle_invalid_command()
{

}

void DriverSession::handle_shutdown()
{

}

void DriverSession::request_matrix()
{

}

void DriverSession::remove_session()
{
//	driver.remove_session();
}

void DriverSession::send_matrix_info(ArrayID matrixID)
{
	write_msg.start(clientID, sessionID, SEND_MATRIX_INFO);
	log->info("Sending back info for matrix {}", matrixID);
	write_msg.write_ArrayInfo(group_driver.get_matrix_info(matrixID));

	flush();
}

void DriverSession::send_layout(vector<vector<uint32_t> > & rows_on_workers)
{
	uint16_t num_workers = rows_on_workers.size();
	uint32_t worker_num_rows, row;

	write_msg.start(clientID, sessionID, SEND_MATRIX_LAYOUT);
	write_msg.write_uint16(num_workers);

	for (uint16_t i = 0; i < num_workers; i++) {
		write_msg.write_uint16(i);
		worker_num_rows = rows_on_workers[i].size();
		write_msg.write_uint32(worker_num_rows);
		for (uint32_t j = 0; j < worker_num_rows; j++) {
			row = rows_on_workers[i][j];
			write_msg.write_uint32(row);
		}
	}

	flush();
}

void DriverSession::send_layout(vector<uint16_t> & row_assignments)
{
	uint32_t num_rows = row_assignments.size();
	uint16_t worker;

	write_msg.start(clientID, sessionID, SEND_MATRIX_LAYOUT);
	write_msg.write_uint32(num_rows);

	for (uint32_t row = 0; row < num_rows; row++)
		write_msg.write_uint16(row_assignments[row]);

	flush();
}


}			// namespace alchemist
