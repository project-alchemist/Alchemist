#ifndef ALCHEMIST__SERVER_HPP
#define ALCHEMIST__SERVER_HPP

#include <ctime>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Session.hpp"
#include "utility/logging.hpp"

namespace alchemist {

class Session;

typedef uint16_t Session_ID;
typedef std::shared_ptr<Session> Session_ptr;
typedef std::deque<Message> Message_queue;

using boost::asio::ip::tcp;
using std::string;


class Server : public std::enable_shared_from_this<Server>
{
public:

	Server(boost::asio::io_context & io_context, const tcp::endpoint & endpoint);
	virtual ~Server() { }

	void deliver(const Session_ptr session, const Message & msg);

	void add_session(Session_ptr session);
	void remove_session(Session_ptr session);

protected:
	string hostname;
	string address;
	uint16_t port;
	Session_ID next_session_ID;

	std::map<Session_ID, Session_ptr> sessions;
	enum { max_recent_msgs = 100 };
	Message_queue recent_msgs;

	tcp::acceptor acceptor;

	void set_log(Log_ptr _log);

	void print_num_sessions();
	int handshake();
	int get_num_sessions();
	void accept_connection();

	virtual int receive_test_string(const Session_ptr, const char *, const uint32_t) = 0;
	virtual int send_test_string(Session_ptr) = 0;

	void print_IP();

	virtual int handle_message(Session_ptr session, const Message & msg) = 0;
private:
	Log_ptr log;
};

typedef std::shared_ptr<Server> Server_ptr;

}

#endif
