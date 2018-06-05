#include "Session.hpp"
#include "DriverSession.hpp"
#include "WorkerSession.hpp"

namespace alchemist {

// =============================================================================================
//                                            Session
// =============================================================================================

Session::Session(tcp::socket _socket, Server & _server)
    : socket(std::move(_socket)), server(_server), ID(0), lm(LibraryManager()), ready(false), admin_privilege(false)
{
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

Session::Session(tcp::socket _socket, Server & _server, uint16_t _ID)
    : socket(std::move(_socket)), server(_server), ID(_ID), lm(LibraryManager()), ready(false), admin_privilege(false)
{
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

Session::Session(tcp::socket _socket, Server & _server, uint16_t _ID, Log_ptr & _log)
    : socket(std::move(_socket)), server(_server), ID(_ID), lm(LibraryManager()), ready(false), admin_privilege(false), log(_log)
{
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

bool Session::get_admin_privilege() const
{
	return admin_privilege;
}

void Session::start()
{
	server.add_session(shared_from_this());
//	send_handshake();
	read_header();
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
			log->info("[Session {}] [{}] Received handshake from client", get_ID(), get_address().c_str());
			return true;
		}
	}

	return false;
}

bool Session::send_response_string()
{
	string test_str = "Alchemist received: '";
	test_str += read_msg.read_string();
	test_str += "'";

	write_msg.start(SEND_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
}

bool Session::send_test_string()
{
	string test_str = "This is a test string from Alchemist";

	write_msg.start(REQUEST_TEST_STRING);
	write_msg.add_string(test_str);
	flush();

	return true;
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

bool Session::load_library()
{
	string library_name = read_msg.read_string();
	string library_path = read_msg.read_string();

	lm.load_library(library_name, library_path);

	return true;
}

bool Session::run_task()
{
	string library_name = read_msg.read_string();

//	lm.run_task(library_name, input_parameters);

	return true;
}

bool Session::unload_library()
{
	string library_name = read_msg.read_string();

	lm.unload_library(library_name);

	return true;
}

void Session::flush()
{
	auto self(shared_from_this());
	boost::asio::async_write(socket,
			boost::asio::buffer(write_msg.data, write_msg.length()),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) write_msg.clear();
			else server.remove_session(shared_from_this());
		});
}

int Session::handle_message()
{

	return 0;
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
		Session(std::move(_socket), _driver), driver(_driver) { }

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver, uint16_t _ID) :
		Session(std::move(_socket), _driver, _ID), driver(_driver) { }

DriverSession::DriverSession(tcp::socket _socket, Driver & _driver, uint16_t _ID, Log_ptr & _log) :
		Session(std::move(_socket), _driver, _ID, _log), driver(_driver) { }


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
			break;
		case YIELD_WORKERS:
			deallocate_workers();
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

bool DriverSession::allocate_workers()
{
	write_msg.start(REQUEST_WORKERS);

	if (read_msg.next_datatype() == UINT16_T && read_msg.next_data_length() == 1) {

		uint16_t num_workers = read_msg.read_uint16();

		if (num_workers > 0) {
			if (num_workers == 1)
				log->info("[Session {}] [{}] Allocating 1 worker", get_ID(), get_address().c_str());
			else
				log->info("[Session {}] [{}] Allocating {} workers", get_ID(), get_address().c_str(), num_workers);

			workers = driver.allocate_workers(DriverSession::shared_from_this(), num_workers);

			write_msg.add_uint16(num_workers);

			for (auto it = workers.begin(); it != workers.end(); it++) {
				write_msg.add_uint16(it->first);
				write_msg.add_string(it->second.hostname);
				write_msg.add_string(it->second.address);
				write_msg.add_uint16(it->second.port);
			}
		}
	}
	else {
		write_msg.add_uint16(0);
		write_msg.add_string("ERROR: Insufficient number of Alchemist workers available");
		write_msg.add_string("       Try again later or request fewer workers");
	}

	flush();

	read_header();

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


void DriverSession::read_header()
{
	read_msg.read_index = 0;
	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.header(), Message::header_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec && read_msg.decode_header()) read_body();
			else driver.remove_session(shared_from_this());
		});
}

void DriverSession::read_body()
{

	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.body(), read_msg.body_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) {
				handle_message();
			}
			else driver.remove_session(shared_from_this());
		});
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

int WorkerSession::handle_message()
{
//	log->info("Received message from Session {} at {}", get_ID(), get_address().c_str());
//	log->info("{}", read_msg.to_string());
//	log->info("{}", read_msg.cc);

	client_command command = read_msg.cc;

	switch (command) {
		case SHUT_DOWN:
//			shut_down();
			break;
	}

	return 0;
}



void WorkerSession::read_header()
{
	read_msg.read_index = 0;
	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.header(), Message::header_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec && read_msg.decode_header()) read_body();
			else worker.remove_session(shared_from_this());
		});
}

void WorkerSession::read_body()
{

	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.body(), read_msg.body_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) {
				handle_message();
			}
			else worker.remove_session(shared_from_this());
		});
}

}			// namespace alchemist
