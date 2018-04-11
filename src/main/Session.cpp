#include "Session.hpp"

namespace alchemist {

Session::Session(tcp::socket _socket, Server & _server)
    : socket(std::move(_socket)), server(_server), ID(0), admin_privilege(false)
{
	address = socket.remote_endpoint().address().to_string();
}

Session::Session(tcp::socket _socket, Server & _server, uint16_t _ID)
    : socket(std::move(_socket)), server(_server), ID(_ID), admin_privilege(false)
{
	address = socket.remote_endpoint().address().to_string();
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
	read_header();
}

//void Session::deliver(const Message & msg)
//{
//	bool write_in_progress = !write_msgs.empty();
//	write_msgs.push_back(msg);
//	if (!write_in_progress) write();
//}

void Session::read_header()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.header(), Message::header_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec && read_msg.decode_header()) read_body();
			else server.remove_session(shared_from_this());
		});
}

void Session::read_body()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket,
			boost::asio::buffer(read_msg.body(), read_msg.body_length),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec) {
				server.deliver(shared_from_this(), read_msg);
				read_header();
			}
			else server.remove_session(shared_from_this());
		});
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

void Session::write(const char * data, std::size_t length, Datatype dt)
{
	write_msg.add(data, length, dt);
}

void Session::flush()
{
	auto self(shared_from_this());
	boost::asio::async_write(socket,
			boost::asio::buffer(write_msg.data, write_msg.length()),
				[this, self](boost::system::error_code ec, std::size_t /*length*/) {
			if (ec) write_msg.clear();
			else server.remove_session(shared_from_this());
		});
}

}			// namespace alchemist
