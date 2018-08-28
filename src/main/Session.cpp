#include "Session.hpp"
#include "DriverSession.hpp"
#include "WorkerSession.hpp"

namespace alchemist {

// =============================================================================================
//                                            Session
// =============================================================================================

Session::Session(tcp::socket _socket, Server & _server)
    : socket(std::move(_socket)), server(_server), ID(0), ready(false), admin_privilege(false)
{
	socket.non_blocking(true);
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

Session::Session(tcp::socket _socket, Server & _server, uint16_t _ID)
    : socket(std::move(_socket)), server(_server), ID(_ID), ready(false), admin_privilege(false)
{
	socket.non_blocking(true);
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

Session::Session(tcp::socket _socket, Server & _server, uint16_t _ID, Log_ptr & _log)
    : socket(std::move(_socket)), server(_server), ID(_ID), ready(false), admin_privilege(false), log(_log)
{
	socket.non_blocking(true);
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

void Session::set_log(Log_ptr _log)
{

	log = _log;
}

void Session::set_ID(Session_ID _ID)
{
	ID = _ID;
}

void Session::set_admin_privilege(bool privilege)
{
	admin_privilege = privilege;
}

Session_ID Session::get_ID() const
{
	return ID;
}

string Session::get_address() const
{
	return address;
}

uint16_t Session::get_port() const
{
	return port;
}

bool Session::get_admin_privilege() const
{
	return admin_privilege;
}

//void Session::deliver(const Message & msg)
//{
//	bool write_in_progress = !write_msgs.empty();
//	write_msgs.push_back(msg);
//	if (!write_in_progress) write();
//}

void Session::wait()
{
	log->info("Waiting for command ...");
}

bool Session::send_handshake()
{
	write_msg.start(HANDSHAKE);
	write_msg.add_uint16(4321);
	write_msg.add_string("DCBA");
	flush();

	return true;
}

bool Session::check_handshake()
{
	if (read_msg.next_datatype() == UINT16_T && read_msg.next_data_length() == 1 && read_msg.read_uint16() == 1234) {
		if (read_msg.next_datatype() == STRING && read_msg.next_data_length() == 4 && read_msg.read_string().compare("ABCD") == 0) {
			log->info("[Session {}] [{}:{}] Received handshake", get_ID(), get_address().c_str(), get_port());
			return true;
		}
	}

	return false;
}

Message & Session::new_message()
{
	write_msg.clear();

	return write_msg;
}

void Session::write_string(const string & data)
{
	write_msg.add_string(data);
}

void Session::write_unsigned_char(const unsigned char & data)
{
	write_msg.add_unsigned_char(data);
}

void Session::write_uint16(const uint16_t & data)
{
	write_msg.add_uint16(data);
}

void Session::write_uint32(const uint32_t & data)
{
	write_msg.add_uint32(data);
}

void Session::write(const char * data, std::size_t length, datatype dt)
{
	write_msg.add(data, length, dt);
}

void Session::read_header()
{
	read_msg.clear();
	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.header(), Message::header_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec && read_msg.decode_header()) read_body();
			else remove_session();
		});
}

void Session::read_body()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.body(), read_msg.body_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) handle_message();
			else remove_session();
		});
}

void Session::flush()
{
//	log->info("{}", write_msg.to_string());
	auto self(shared_from_this());
	boost::asio::async_write(socket,
			boost::asio::buffer(write_msg.data, write_msg.length()),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) write_msg.clear();
			else remove_session();
		});
}

//int Session::handle_message()
//{
////	log->info("Received message from Session {} at {}", get_ID(), get_address().c_str());
////	log->info("{}", read_msg.to_string());
////	log->info("{}", read_msg.cc);
//
//	client_command command = read_msg.cc;
//
//	switch (command) {
//		case SHUT_DOWN:
////			shut_down();
//			break;
//		case HANDSHAKE:
//			if (check_handshake()) send_handshake();
//			wait();
//			read_header();
//			break;
//		case REQUEST_TEST_STRING:
//			send_test_string();
//			wait();
//			read_header();
//			break;
//		case SEND_TEST_STRING:
//			send_response_string();
//			wait();
//			read_header();
//			break;
//		case ASSIGN_WORKERS:
//			assign_workers();
//			break;
//		case SEND_ASSIGNED_WORKERS_INFO:
////			send_assigned_workers_info();
//			break;
//		case LIST_ALL_WORKERS:
//			list_all_workers();
//			break;
//		case LIST_ACTIVE_WORKERS:
//			list_active_workers();
//			break;
//		case LIST_INACTIVE_WORKERS:
//			list_inactive_workers();
//			break;
//		case LIST_ASSIGNED_WORKERS:
//			list_assigned_workers();
//			break;
//	}
//
//	return 0;
//}
//
//void Session::assign_workers()
//{
//	write_msg.start(ASSIGN_WORKERS);
//
//	if (read_msg.next_datatype() == UINT16_T && read_msg.next_data_length() == 1) {
//
//		uint16_t num_workers = read_msg.read_uint16();
//
//		log->info("Assigning {} workers to Session {} at {}", num_workers, get_ID(), get_address().c_str());
//
//		std::map<Worker_ID, WorkerInfo> workers = server.assign_workers(DriverSession::shared_from_this(), num_workers);
//
//		auto it = workers.begin();
//
//		for ( ; it != workers.end(); it++) {
//			write_msg.add_string(it->second.hostname);
//			write_msg.add_string(it->second.address);
//			write_msg.add_uint16(it->second.port);
//		}
//	}
//
//	flush();
//}
//
//bool Session::list_all_workers()
//{
//	log->info("Sending list of all workers to Session {} at {}", get_ID(), get_address().c_str());
//
//	write_msg.start(LIST_ALL_WORKERS);
//	write_msg.add_string(server.list_workers());
//	flush();
//
//	read_header();
//
//	return true;
//}
//
//bool Session::list_active_workers()
//{
//	log->info("Sending list of active workers to Session {} at {}", get_ID(), get_address().c_str());
//
//	write_msg.start(LIST_ACTIVE_WORKERS);
//	write_msg.add_string(server.list_active_workers());
//	flush();
//
//	read_header();
//
//	return true;
//}
//
//bool Session::list_inactive_workers()
//{
//	log->info("Sending list of inactive workers to Session {} at {}", get_ID(), get_address().c_str());
//
//	write_msg.start(LIST_INACTIVE_WORKERS);
//	write_msg.add_string(server.list_inactive_workers());
//	flush();
//
//	read_header();
//
//	return true;
//}
//
//bool Session::list_assigned_workers()
//{
//	log->info("Sending list of assigned workers to Session {} at {}", get_ID(), get_address().c_str());
//
//	write_msg.start(LIST_INACTIVE_WORKERS);
//	write_msg.add_string(server.list_assigned_workers(DriverSession::shared_from_this()));
//	flush();
//
//	read_header();
//
//	return true;
//}

// =============================================================================================
//                                         DriverSession
// =============================================================================================

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver) :
		Session(std::move(_socket), _driver), driver(_driver), num_client_workers(0) { }

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver, uint16_t _ID) :
		Session(std::move(_socket), _driver, _ID), driver(_driver), num_client_workers(0) { }

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver, uint16_t _ID, Log_ptr & _log) :
		Session(std::move(_socket), _driver, _ID, _log), driver(_driver), num_client_workers(0) { }

void DriverSession::start()
{
	driver.add_session(shared_from_this());
//	send_handshake();
	read_header();
}

int DriverSession::handle_message()
{
//	log->info("Received message from Session {} at {}", get_ID(), get_address().c_str());
//	log->info("{}", read_msg.to_string());
//	log->info("{}", read_msg.cc);

	client_command command = read_msg.cc;

	switch (command) {
		case SHUT_DOWN:
//			shut_down();
			break;
		case HANDSHAKE:
			if (check_handshake()) send_handshake();
			read_header();
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
			driver.print_workers();

			write_msg.start(YIELD_WORKERS);
			write_msg.add_string("Alchemist workers have been deallocated");
				flush();

				read_header();
//			deallocate_workers();
			break;
		case SEND_ASSIGNED_WORKERS_INFO:
//			send_assigned_workers_info();
			break;
		case LIST_ALL_WORKERS:
			list_all_workers();
			break;
		case LIST_ACTIVE_WORKERS:
			list_active_workers();
			break;
		case LIST_INACTIVE_WORKERS:
			list_inactive_workers();
			break;
		case LIST_ASSIGNED_WORKERS:
			list_assigned_workers();
			break;
		case MATRIX_INFO:
			new_matrix();
			break;
		case LOAD_LIBRARY:
			load_library();
			break;
		case RUN_TASK:
			run_task();
			break;
		case UNLOAD_LIBRARY:
			unload_library();
			break;
	}

	return 0;
}

void DriverSession::remove_session()
{
	driver.remove_session(shared_from_this());
}

void DriverSession::send_matrix_info(Matrix_ID matrix_ID)
{
	write_msg.start(MATRIX_INFO);
	write_msg.add_uint16(matrix_ID);

	driver.determine_row_assignments(matrix_ID);
	vector<uint16_t> & row_assignments = driver.get_row_assignments(matrix_ID);

	uint32_t num_rows = row_assignments.size();
	uint16_t worker;

	write_msg.add_uint32(num_rows);

	for (uint32_t row = 0; row < num_rows; row++)
		write_msg.add_uint16(row_assignments[row]);

	flush();
}

void DriverSession::send_layout(vector<vector<uint32_t> > & rows_on_workers)
{
	uint16_t num_workers = rows_on_workers.size();
	uint32_t worker_num_rows, row;

	write_msg.start(MATRIX_LAYOUT);
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

	write_msg.start(MATRIX_LAYOUT);
	write_msg.add_uint32(num_rows);

	for (uint32_t row = 0; row < num_rows; row++)
		write_msg.add_uint16(row_assignments[row]);

	flush();
}

void DriverSession::new_matrix()
{
	unsigned char type   = read_msg.read_unsigned_char();
	unsigned char layout = read_msg.read_unsigned_char();
	uint32_t num_rows    = read_msg.read_uint32();
	uint32_t num_cols    = read_msg.read_uint32();

	send_matrix_info(driver.new_matrix(type, layout, num_rows, num_cols));
}

bool DriverSession::receive_client_info()
{
	num_client_workers = read_msg.read_uint16();
	log_dir = read_msg.read_string();

	string response_str = "Logs saved to: ";
	response_str += log_dir;

	write_msg.start(CLIENT_INFO);
	write_msg.add_string(response_str);
	flush();

	log->info(response_str);

	return true;
}

bool DriverSession::send_test_string()
{
	string test_str = "This is a test string from Alchemist driver";

	write_msg.start(REQUEST_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
}

bool DriverSession::send_response_string()
{
	string test_str = "Alchemist driver received: '";
	test_str += read_msg.read_string();
	test_str += "'";

	write_msg.start(SEND_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
}

bool DriverSession::load_library()
{
	string library_name = read_msg.read_string();
	string library_path = read_msg.read_string();

	log->info("Loading library {} at {}", library_name, library_path);

	int result = driver.load_library(library_name, library_path);

	write_msg.start(LOAD_LIBRARY);
	if (result == 0)
		write_msg.add_string("Library successfully loaded");
	else
		write_msg.add_string("Could not load library");

	flush();

	return true;
}

bool DriverSession::run_task()
{
	string library_name = read_msg.read_string();

//	lm.run_task(library_name, input_parameters);

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
	write_msg.start(REQUEST_WORKERS);

	if (read_msg.next_datatype() == UINT16_T && read_msg.next_data_length() == 1) {

		uint16_t num_workers = read_msg.read_uint16();

		if (num_workers > 0) {

			std::stringstream list_of_alchemist_workers;
			char buffer[4];

			list_of_alchemist_workers << "[Session " << get_ID() << "] [" << get_address().c_str() << "] ";

			if (num_workers == 1)
				list_of_alchemist_workers << "Allocating 1 worker:", get_ID(), get_address().c_str();
			else
				list_of_alchemist_workers << "Allocating " << num_workers << " workers:";
			list_of_alchemist_workers << std::endl;

			workers = driver.allocate_workers(DriverSession::shared_from_this(), num_workers);

			write_msg.add_uint16(num_workers);

			for (auto it = workers.begin(); it != workers.end(); it++) {
				sprintf(buffer, "%03d", it->first);
				list_of_alchemist_workers << SPACE;
				list_of_alchemist_workers << "    Worker-" << string(buffer) << " running on " << it->second.hostname << " ";
				list_of_alchemist_workers << it->second.address << ":" << it->second.port << std::endl;

				write_msg.add_uint16(it->first);
				write_msg.add_string(it->second.hostname);
				write_msg.add_string(it->second.address);
				write_msg.add_uint16(it->second.port);
			}

			auto layout_rr = driver.prepare_data_layout_table(num_workers, num_client_workers);

			write_msg.add_uint16(num_client_workers);
			for (int i = 0; i < num_client_workers; i++) {
				for (int j = 0; j < num_workers; j++) {
					write_msg.add_float(layout_rr[i][j][0]);
					write_msg.add_float(layout_rr[i][j][1]);
				}
			}

			log->info(list_of_alchemist_workers.str());
		}
		else {
			write_msg.add_uint16(0);
			write_msg.add_string("ERROR: Insufficient number of Alchemist workers available");
			write_msg.add_string("       Try again later or request fewer workers");
		}
	}

	flush();

	return true;
}

bool DriverSession::deallocate_workers()
{
	write_msg.start(YIELD_WORKERS);

	auto num_workers = workers.size();

	if (num_workers == 1)
		log->info("[Session {}] [{}] Deallocating 1 worker", get_ID(), get_address().c_str());
	else
		log->info("[Session {}] [{}] Deallocating {} workers", get_ID(), get_address().c_str(), num_workers);

	driver.deallocate_workers(DriverSession::shared_from_this());

	workers.clear();

	write_msg.add_string("Alchemist workers have been deallocated");
	flush();

	read_header();

	return true;
}

bool DriverSession::list_all_workers()
{
	log->info("[Session {}] [{}] Sending list of all workers", get_ID(), get_address().c_str());

	write_msg.start(LIST_ALL_WORKERS);
	write_msg.add_string(driver.list_workers());
	flush();

	read_header();

	return true;
}

bool DriverSession::list_active_workers()
{
	log->info("[Session {}] [{}] Sending list of active workers", get_ID(), get_address().c_str());

	write_msg.start(LIST_ACTIVE_WORKERS);
	write_msg.add_string(driver.list_active_workers());
	flush();

	read_header();

	return true;
}

bool DriverSession::list_inactive_workers()
{
	log->info("[Session {}] [{}] Sending list of inactive workers", get_ID(), get_address().c_str());

	write_msg.start(LIST_INACTIVE_WORKERS);
	write_msg.add_string(driver.list_inactive_workers());
	flush();

	read_header();

	return true;
}

bool DriverSession::list_assigned_workers()
{
	log->info("[Session {}] [{}] Sending list of assigned workers", get_ID(), get_address().c_str());

	write_msg.start(LIST_INACTIVE_WORKERS);
	write_msg.add_string(driver.list_allocated_workers(DriverSession::shared_from_this()));
	flush();

	read_header();

	return true;
}

// =============================================================================================
//                                         WorkerSession
// =============================================================================================

WorkerSession::WorkerSession(tcp::socket _socket, Worker & _worker) :
		Session(std::move(_socket), _worker), worker(_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, Worker & _worker, uint16_t _ID) :
		Session(std::move(_socket), _worker, _ID), worker(_worker) { }

WorkerSession::WorkerSession(tcp::socket _socket, Worker & _worker, uint16_t _ID, Log_ptr & _log) :
		Session(std::move(_socket), _worker, _ID, _log), worker(_worker) { }

void WorkerSession::start()
{
	worker.add_session(shared_from_this());
//	send_handshake();
	read_header();
}

void WorkerSession::remove_session()
{
	worker.remove_session(shared_from_this());
}

int WorkerSession::handle_message()
{
//	log->info("Received message from Session {} at {}:{}", get_ID(), get_address().c_str(), get_port());
//	log->info("{}", read_msg.to_string());

	client_command command = read_msg.cc;

//	log->info("{}", command);

	switch (command) {
		case SHUT_DOWN:
//			shut_down();
			break;
		case HANDSHAKE:
			if (check_handshake()) send_handshake();
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
		case MATRIX_BLOCK:
			receive_data();
			read_header();
			break;
	}

	return 0;
}

bool WorkerSession::receive_data()
{
	float value;
	uint32_t num_blocks, row_start, row_end, col_start, col_end;

	Matrix_ID ID = read_msg.read_uint16();
	num_blocks = read_msg.read_uint32();

//	std::vector<std::vector<float> > data;

	for (auto block = 0; block < num_blocks; block++) {

		row_start = read_msg.read_uint32();
		row_end   = read_msg.read_uint32();
		col_start = read_msg.read_uint32();
		col_end   = read_msg.read_uint32();

//		log->info("Matrix {} block dimensions: {} {} {} {}", ID, row_start, row_end, col_start, col_end);

//		data = std::vector<std::vector<float> >(row_end-row_start+1, std::vector<float>(col_end-col_start+1));

		for (auto i = row_start; i <= row_end; i++)
			for (auto j = col_start; j <= col_end; j++) {
//				value = read_msg.read_float();

				worker.set_value(ID, i, j, read_msg.read_float());
//				data[i-row_start][j-col_start] = value;
			}

//		for (auto i = 0; i <= row_end-row_start; i++) {
//			for (auto j = 0; j <= col_end-col_start; j++)
//				std::cout << data[i][j] << " ";
//			std::cout << std::endl;
//		}

		log->info("[Session {}] [{}:{}] Matrix {}: Received matrix block (rows {}-{}, columns {}-{})", get_ID(), get_address().c_str(), get_port(), ID, row_start, row_end, col_start, col_end);
	}

//	worker.print_data(ID);

	char buffer[4];
	std::stringstream test_str;

	sprintf(buffer, "%03d", worker.get_ID());
	test_str << "Alchemist worker " << buffer << " received " << num_blocks << " blocks for matrix " << ID;

	write_msg.start(MATRIX_BLOCK);
	write_msg.add_string(test_str.str());
	flush();

	return true;
}

bool WorkerSession::send_test_string()
{
	char buffer[4];
	std::stringstream test_str;

	sprintf(buffer, "%03d", worker.get_ID());
	test_str << "This is a test string from Alchemist worker " << buffer;

	write_msg.start(REQUEST_TEST_STRING);
	write_msg.add_string(test_str.str());
	flush();

	return true;
}

bool WorkerSession::send_response_string()
{
	string test_str = "Alchemist worker received: '";
	test_str += read_msg.read_string();
	test_str += "'";

	write_msg.start(SEND_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
}


}			// namespace alchemist
