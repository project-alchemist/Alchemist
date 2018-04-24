#ifndef ALCHEMIST__SESSION_HPP
#define ALCHEMIST__SESSION_HPP



#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include "Message.hpp"
//#include "data_stream.hpp"
#include "Server.hpp"
#include "utility/logging.hpp"

#if BOOST_VERSION < 106600
typedef boost::asio::io_service io_context;
#else
typedef boost::asio::io_context io_context;
#endif

namespace alchemist {

using boost::asio::ip::tcp;
using std::string;

typedef uint16_t Session_ID;
typedef std::deque<Message> Message_queue;

class Server;


class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket, Server &);
	Session(tcp::socket, Server &, uint16_t);

	void start();

	void set_log(Log_ptr _log);
	void set_ID(Session_ID _ID);
	void set_admin_privilege(bool privilege);

	Session_ID get_ID() const;
	string get_address() const;
	bool get_admin_privilege() const;

	void write_string(const string & data);
	void write_unsigned_char(const unsigned char & data);
	void write_uint16(const uint16_t & data);
	void write_uint32(const uint32_t & data);

	void write(const char * data, std::size_t length, Datatype dt);
	void flush();

private:
	bool admin_privilege;
	Log_ptr log;

	void read_header();
	void read_body();

	tcp::socket socket;
	Server & server;
	Message read_msg;
	Message write_msg;
	Message_queue write_msgs;

	Session_ID ID;
	string address;
};

typedef std::shared_ptr<Session> Session_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__SESSION_HPP
