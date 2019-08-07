#include "Session.hpp"

namespace alchemist {

// =============================================================================================
//                                            Session
// =============================================================================================

Session::Session(tcp::socket _socket)
    : socket(std::move(_socket)), sessionID(0), ready(false), admin_privilege(false)
{
	socket.non_blocking(true);
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

Session::Session(tcp::socket _socket, SessionID _sessionID, ClientID _clientID)
    : socket(std::move(_socket)), sessionID(_sessionID), clientID(_clientID), ready(false), admin_privilege(false)
{
	socket.non_blocking(true);
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

Session::Session(tcp::socket _socket, SessionID _sessionID, ClientID _clientID, Log_ptr & _log)
    : socket(std::move(_socket)), sessionID(_sessionID), clientID(_clientID), ready(false), admin_privilege(false), log(_log)
{
	socket.non_blocking(true);
	address = socket.remote_endpoint().address().to_string();
	port = socket.remote_endpoint().port();
}

void Session::set_client_language(client_language _cl)
{
	cl = _cl;
	write_msg.set_client_language(cl);
	read_msg.set_client_language(cl);
}

void Session::set_log(Log_ptr _log)
{

	log = _log;
}

void Session::setID(SessionID _sessionID)
{
	sessionID = _sessionID;
}

void Session::set_admin_privilege(bool privilege)
{
	admin_privilege = privilege;
}

SessionID Session::get_sessionID() const
{
	return sessionID;
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

SessionID Session::assign_sessionID()
{
	return ++sessionID;
}

void Session::wait()
{
	log->info(string("Waiting for command ..."));
}

string Session::preamble()
{
	return client_preamble() + " " + session_preamble();
}

string Session::address_preamble()
{
	std::stringstream ss;

	ss << "[" << get_address().c_str() << "]";

	return ss.str();
}

string Session::client_preamble()
{
	std::stringstream ss;

	ss << "[Client " << clientID << " (" << address.c_str() << ":" << port << ")]";

	return ss.str();
}

string Session::session_preamble()
{
	std::stringstream ss;

	ss << "[Session " << sessionID << "]";

	return ss.str();
}

bool Session::handle_handshake()
{
	cl = read_msg.read_client_language();
	write_msg.set_client_language(cl);
	read_msg.set_client_language(cl);

	if (read_msg.read_uint16() == 1234 && read_msg.read_string().compare(string("ABCD")) == 0) {
		double d1 = read_msg.read_double();
		if (d1 != 1.11) {
			read_msg.reverse_floats = true;
			write_msg.reverse_floats = true;
		}
		log->info("{} Reverse floats set to {}", preamble(), read_msg.reverse_floats);
		double d2 = read_msg.read_double();
		if (d2 == 2.22) {
			double * temp = new double[12];
			for (auto i = 0; i < 12; i++) temp[i] = 1.11*(i+3);
			MatrixBlock_ptr block = read_msg.read_MatrixBlock();

			if (read_msg.compare_matrix_block(block, temp)) {

				uint32_t buffer_length = read_msg.read_uint32();

				log->info("{} Received handshake", preamble());
				log->info("{} Client Language is {}", preamble(), get_client_language_name(cl));

				set_message_buffer_lengths(buffer_length);

				return valid_handshake();
			}
		}
	}

	invalid_handshake();

	return false;
}

void Session::set_message_buffer_lengths(uint32_t buffer_length)
{
	uint32_t previous_buffer_length = read_msg.get_buffer_length();
	read_msg.set_buffer_length(buffer_length);
	uint32_t current_buffer_length = read_msg.get_buffer_length();

	log->info("{} Input message buffer length changed from {} to {} bytes", preamble(), previous_buffer_length, current_buffer_length);

	previous_buffer_length = write_msg.get_buffer_length();
	write_msg.set_buffer_length(buffer_length);
	current_buffer_length = write_msg.get_buffer_length();

	log->info("{} Output message buffer length changed from {} to {} bytes", preamble(), previous_buffer_length, current_buffer_length);
}

bool Session::valid_handshake()
{
	assign_sessionID();

	write_msg.start(clientID, sessionID, HANDSHAKE);

	write_msg.write_uint16(4321);
	write_msg.write_string(string("DCBA"));
	write_msg.write_double(3.33);

	flush();

	return true;
}

bool Session::invalid_handshake()
{
	write_msg.start(clientID, sessionID, HANDSHAKE);
	write_msg.write_error_code(ERR_INVALID_HANDSHAKE);

	flush();

	log->info("{} Invalid handshake format", session_preamble());

	return true;
}

Message & Session::new_message()
{
	write_msg.clear();

	return write_msg;
}

void Session::read_header()
{
	read_msg.clear();
	auto self(shared_from_this());
	asio::async_read(socket,
			asio::buffer(read_msg.header(), Message::header_length),
				[this, self](error_code ec, std::size_t /*length*/) {
			if (!ec) { read_msg.decode_header(); read_body(); }
			else remove_session();
		});
}

void Session::read_body()
{
	auto self(shared_from_this());
	asio::async_read(socket,
			asio::buffer(read_msg.body(), read_msg.body_length),
				[this, self](error_code ec, std::size_t /*length*/) {
			if (!ec) handle_message();
			else remove_session();
		});
}

void Session::flush()
{
	write_msg.finish();
#ifdef DEBUG
//	log->info("OUT: {}", write_msg.to_string());
#endif
	auto self(shared_from_this());
	asio::async_write(socket,
			asio::buffer(write_msg.header(), write_msg.length()),
				[this, self](error_code ec, std::size_t /*length*/) {
			if (!ec) write_msg.clear();
			else remove_session();
		});
}



//int Session::handle_message()
//{
////	log->info("Received message from Session {} at {}", getID(), get_address().c_str());
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
//		log->info("Assigning {} workers to Session {} at {}", num_workers, getID(), get_address().c_str());
//
//		std::map<WorkerID, WorkerInfo> workers = server.assign_workers(DriverSession::shared_from_this(), num_workers);
//
//		auto it = workers.begin();
//
//		for ( ; it != workers.end(); it++) {
//			write_msg.write_string(it->second.hostname);
//			write_msg.write_string(it->second.address);
//			write_msg.write_uint16(it->second.port);
//		}
//	}
//
//	flush();
//}
//
//bool Session::list_all_workers()
//{
//	log->info("Sending list of all workers to Session {} at {}", getID(), get_address().c_str());
//
//	write_msg.start(LIST_ALL_WORKERS);
//	write_msg.write_string(server.list_workers());
//	flush();
//
//	read_header();
//
//	return true;
//}
//
//bool Session::list_active_workers()
//{
//	log->info("Sending list of active workers to Session {} at {}", getID(), get_address().c_str());
//
//	write_msg.start(LIST_ACTIVE_WORKERS);
//	write_msg.write_string(server.list_active_workers());
//	flush();
//
//	read_header();
//
//	return true;
//}
//
//bool Session::list_inactive_workers()
//{
//	log->info("Sending list of inactive workers to Session {} at {}", getID(), get_address().c_str());
//
//	write_msg.start(LIST_INACTIVE_WORKERS);
//	write_msg.write_string(server.list_inactive_workers());
//	flush();
//
//	read_header();
//
//	return true;
//}
//
//bool Session::list_assigned_workers()
//{
//	log->info("Sending list of assigned workers to Session {} at {}", getID(), get_address().c_str());
//
//	write_msg.start(LIST_INACTIVE_WORKERS);
//	write_msg.write_string(server.list_assigned_workers(DriverSession::shared_from_this()));
//	flush();
//
//	read_header();
//
//	return true;
//}


}			// namespace alchemist
