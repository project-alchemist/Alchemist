#include "DriverSession.hpp"

namespace alchemist {

// =============================================================================================
//                                         DriverSession
// =============================================================================================

DriverSession::DriverSession(tcp::socket _socket, GroupDriver & _group_driver) :
		Session(std::move(_socket)), group_driver(_group_driver), num_group_workers(0) { }

DriverSession::DriverSession(tcp::socket _socket, GroupDriver & _group_driver, Client_ID _client_ID) :
		Session(std::move(_socket), 0, _client_ID), group_driver(_group_driver), num_group_workers(0) { }

DriverSession::DriverSession(tcp::socket _socket, GroupDriver & _group_driver, Client_ID _client_ID, Log_ptr & _log) :
		Session(std::move(_socket), 0, _client_ID, _log), group_driver(_group_driver), num_group_workers(0) { }

void DriverSession::start()
{
	log->info("DriverSession: START");
	log->info("{} Connection established", preamble());
//	driver.add_session(shared_from_this());
	read_header();
}

int DriverSession::handle_message()
{
//	log->info("Received message from Session {} at {}", get_ID(), get_address().c_str());
//	log->info("{}", read_msg.to_string());
//	log->info("{}", read_msg.cc);

	client_command command = read_msg.cc;

	if (command == HANDSHAKE) {
		check_handshake();
		read_header();
	}
	else {
		if (client_ID == read_msg.client_ID && ID == read_msg.session_ID) {
			switch (command) {
				case SHUT_DOWN:
		//			shut_down();
					break;
				case CLIENT_INFO:
					receive_client_info();
					read_header();
					break;
				case REQUEST_TEST_STRING:
					send_test_string();
					read_header();
					break;
				case SEND_TEST_STRING:
					send_response_string();
					read_header();
					break;
				case REQUEST_WORKERS:
					allocate_workers();
					read_header();
					break;
				case YIELD_WORKERS:
	//				driver.print_workers();

					deallocate_workers();
					write_msg.start(client_ID, ID, YIELD_WORKERS);
					write_msg.add_string("Alchemist workers have been deallocated");
						flush();

						read_header();
					break;
				case SEND_ASSIGNED_WORKERS_INFO:
		//			send_assigned_workers_info();
					break;
				case LIST_ALL_WORKERS:
					list_all_workers();
					read_header();
					break;
				case LIST_ACTIVE_WORKERS:
					list_active_workers();
					read_header();
					break;
				case LIST_INACTIVE_WORKERS:
					list_inactive_workers();
					read_header();
					break;
				case LIST_ASSIGNED_WORKERS:
					list_assigned_workers();
					read_header();
					break;
				case MATRIX_INFO:
					new_matrix();
					read_header();
					break;
				case LOAD_LIBRARY:
					load_library();
					read_header();
					break;
				case RUN_TASK:
					run_task();
					read_header();
					break;
				case UNLOAD_LIBRARY:
					unload_library();
					read_header();
					break;
				}
		}
		else log->error("{} Error in DriverSession: Wrong ID {}:{}", preamble(), read_msg.client_ID, read_msg.session_ID);
	}

	return 0;
}

void DriverSession::request_matrix()
{

}

void DriverSession::remove_session()
{
//	driver.remove_session();
}

void DriverSession::send_matrix_info(Matrix_ID matrix_ID)
{
	write_msg.start(client_ID, ID, MATRIX_INFO);
	log->info("Sending back info for matrix {}", matrix_ID);
	write_msg.add_uint16(matrix_ID);

	group_driver.determine_row_assignments(matrix_ID);
	vector<uint16_t> & row_assignments = group_driver.get_row_assignments(matrix_ID);

	uint64_t num_rows = row_assignments.size();
	uint64_t num_cols = group_driver.get_num_cols(matrix_ID);
//	uint16_t worker;

	write_msg.add_uint64(num_rows);
	write_msg.add_uint64(num_cols);

	for (uint32_t row = 0; row < num_rows; row++)
		write_msg.add_uint16(row_assignments[row]);

	flush();
}

void DriverSession::send_layout(vector<vector<uint32_t> > & rows_on_workers)
{
	uint16_t num_workers = rows_on_workers.size();
	uint32_t worker_num_rows, row;

	write_msg.start(client_ID, ID, MATRIX_LAYOUT);
	write_msg.add_uint16(num_workers);

	for (uint16_t i = 0; i < num_workers; i++) {
		write_msg.add_uint16(i);
		worker_num_rows = rows_on_workers[i].size();
		write_msg.add_uint32(worker_num_rows);
		for (uint32_t j = 0; j < worker_num_rows; j++) {
			row = rows_on_workers[i][j];
			write_msg.add_uint32(row);
		}
	}

	flush();
}

void DriverSession::send_layout(vector<uint16_t> & row_assignments)
{
	uint32_t num_rows = row_assignments.size();
	uint16_t worker;

	write_msg.start(client_ID, ID, MATRIX_LAYOUT);
	write_msg.add_uint32(num_rows);

	for (uint32_t row = 0; row < num_rows; row++)
		write_msg.add_uint16(row_assignments[row]);

	flush();
}

void DriverSession::new_matrix()
{
	unsigned char type   = read_msg.read_unsigned_char();
	unsigned char layout = read_msg.read_unsigned_char();
	uint64_t num_rows    = read_msg.read_uint64();
	uint64_t num_cols    = read_msg.read_uint64();

	send_matrix_info(group_driver.new_matrix(type, layout, num_rows, num_cols));
}

bool DriverSession::receive_client_info()
{
	Client_ID _client_ID =
	num_group_workers = read_msg.read_uint16();
	log_dir = read_msg.read_string();

	string response_str = "Logs saved to: ";
	response_str += log_dir;

	write_msg.start(client_ID, ID, CLIENT_INFO);
	write_msg.add_string(response_str);
	flush();

	log->info(response_str);

	return true;
}

bool DriverSession::send_test_string()
{
	string test_str = "This is a test string from Alchemist driver";

	write_msg.start(client_ID, ID, REQUEST_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
}

bool DriverSession::send_response_string()
{
	string test_str = "Alchemist driver received: '";
	test_str += read_msg.read_string();
	test_str += "'";

	write_msg.start(client_ID, ID, SEND_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
}

bool DriverSession::load_library()
{
	group_driver.load_library(read_msg.read_string(), read_msg.read_string());

	uint16_t temp = 1;
	write_msg.start(client_ID, ID, LOAD_LIBRARY);
	write_msg.add_uint16(temp);
	flush();

	return true;
}

bool DriverSession::run_task()
{
	string name = read_msg.read_string();
	Matrix_ID matrix_ID = read_msg.read_uint16();
	uint32_t rank = read_msg.read_uint32();
	uint8_t method = read_msg.read_uint8();

	group_driver.run_task(name, matrix_ID, rank, method);

	write_msg.start(client_ID, ID, RUN_TASK);

	for (int i = 0; i < 3; i++) {
		matrix_ID++;
		write_msg.add_uint16(matrix_ID);

		group_driver.determine_row_assignments(matrix_ID);
		vector<uint16_t> & row_assignments = group_driver.get_row_assignments(matrix_ID);

		uint64_t num_rows = row_assignments.size();
		uint64_t num_cols = group_driver.get_num_cols(matrix_ID);
		uint16_t worker;

		write_msg.add_uint64(num_rows);
		write_msg.add_uint64(num_cols);

		for (uint32_t row = 0; row < num_rows; row++)
			write_msg.add_uint16(row_assignments[row]);
	}
	flush();

	return true;
}

bool DriverSession::unload_library()
{
//	string library_name = read_msg.read_string();
//
//	driver.unload_library(library_name);

	return true;
}

bool DriverSession::allocate_workers()
{
	write_msg.start(client_ID, ID, REQUEST_WORKERS);

	uint16_t num_requested_workers = read_msg.read_uint16();

	if (num_requested_workers > 0) {

		map<Worker_ID, WorkerInfo> allocated_group = group_driver.allocate_workers(num_requested_workers);

		if (allocated_group.size() > 0) {
			uint16_t group_size = allocated_group.size();
			write_msg.add_uint16(group_size);
			for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
				write_msg.add_uint16(it->first);
				write_msg.add_string(it->second.hostname);
				write_msg.add_string(it->second.address);
				write_msg.add_uint16(it->second.port);
			}
		}
		else {
			write_msg.add_uint16(0);
			write_msg.add_string("ERROR: Insufficient number of Alchemist workers available");
			write_msg.add_string("       Try again later or request fewer workers");
		}


//		std::stringstream list_of_alchemist_workers;
//		char buffer[4];
//
//		list_of_alchemist_workers << session_preamble();
//
//		if (num_workers == 1)
//			list_of_alchemist_workers << "Allocating 1 worker:", get_ID(), get_address().c_str();
//		else
//			list_of_alchemist_workers << "Allocating " << num_workers << " workers:";
//		list_of_alchemist_workers << std::endl;

//		auto workers = driver.allocate_workers(client_ID, num_workers);

//		add_uint16(num_workers);
//		const int group_IDs[num_workers+1];

//		group_IDs[0] = 0;
//
//		for (auto it = workers.begin(); it != workers.end(); it++) {
//			sprintf(buffer, "%03d", it->first);
//			list_of_alchemist_workers << SPACE;
//			list_of_alchemist_workers << "    Worker-" << string(buffer) << " running on " << it->second.hostname << " ";
//			list_of_alchemist_workers << it->second.address << ":" << it->second.port << std::endl;
//
////			group_IDs[i+1] = peer_ID;
//
//			add_uint16(it->first);
//			add_string(it->second.hostname);
//			add_string(it->second.address);
//			add_uint16(it->second.port);
//		}

//			auto layout_rr = driver.prepare_data_layout_table(num_workers, num_group_workers);

//			write_msg.add_uint16(num_group_workers);
//			for (int i = 0; i < num_group_workers; i++) {
//				for (int j = 0; j < num_workers; j++) {
//					write_msg.add_float(layout_rr[i][j][0]);
//					write_msg.add_float(layout_rr[i][j][1]);
//				}
//			}

//		log->info(list_of_alchemist_workers.str());

//
//		MPI_Status status;
//
//		MPI_Recv(&session_ID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//
//		uint16_t num_peers = 0;
//
//		MPI_Recv(&num_peers, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//
//		uint16_t peer_ID;
//		const int group_IDs[num_peers+1];
//		const int group_peer_IDs[num_peers];
//
//		group_IDs[0] = 0;			// First member is the driver
//		for (uint16_t i = 0; i < num_peers; i++) {
//			MPI_Recv(&peer_ID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//			group_IDs[i+1] = peer_ID;
//			group_peer_IDs[i] = peer_ID;
//		}
//
//		MPI_Group world_group;
//		MPI_Group temp_group;
//
//		MPI_Comm_group(world, &world_group);
//
//		MPI_Group_incl(world_group, (int) (num_peers+1), group_IDs, &temp_group);
//		MPI_Comm_create_group(world, temp_group, 0, &group);			// Create a new communicator based on the group_IDs
//




	}
	else {
		write_msg.add_uint16(0);
		write_msg.add_string("ERROR: Number of requested workers must be positive");
	}


	flush();

	return true;
}

bool DriverSession::deallocate_workers()
{
	write_msg.start(client_ID, ID, YIELD_WORKERS);

//	auto num_workers = workers.size();
//
//	if (num_workers == 1)
//		log->info("[Session {}] [{}] Deallocating 1 worker", get_ID(), get_address().c_str());
//	else
//		log->info("[Session {}] [{}] Deallocating {} workers", get_ID(), get_address().c_str(), num_workers);

//	driver.deallocate_workers(DriverSession::shared_from_this());


	group_driver.deallocate_workers();

//	add_string("Alchemist workers have been deallocated");
//	flush();

	return true;
}

bool DriverSession::list_all_workers()
{
	log->info("[Session {}] [{}] Sending list of all workers", get_ID(), get_address().c_str());

	write_msg.start(client_ID, ID, LIST_ALL_WORKERS);
	write_msg.add_uint16(group_driver.get_num_workers());
	write_msg.add_string(group_driver.list_workers());
	flush();

	return true;
}

bool DriverSession::list_active_workers()
{
	log->info("[Session {}] [{}] Sending list of active workers", get_ID(), get_address().c_str());

//	write_msg.start(LIST_ACTIVE_WORKERS);
//	add_string(driver.list_active_workers());
//	flush();

	return true;
}

bool DriverSession::list_inactive_workers()
{
	log->info("[Session {}] [{}] Sending list of inactive workers", get_ID(), get_address().c_str());

//	write_msg.start(LIST_INACTIVE_WORKERS);
//	add_string(driver.list_inactive_workers());
//	flush();

	return true;
}

bool DriverSession::list_assigned_workers()
{
	log->info("[Session {}] [{}] Sending list of assigned workers", get_ID(), get_address().c_str());

//	write_msg.start(LIST_INACTIVE_WORKERS);
//	add_string(driver.list_allocated_workers(DriverSession::shared_from_this()));
//	flush();

	return true;
}


}			// namespace alchemist
