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


class Server// : public std::enable_shared_from_this<Server>
{
public:


	Server();
	Server(io_context & _io_context, const tcp::endpoint & endpoint);
	virtual ~Server() { }

//	void deliver(const Session_ptr session, Message & msg);

	void add_session(Session_ptr session);
	void remove_session(Session_ptr session);

	virtual int start() = 0;

//	virtual string list_workers() = 0;
//	virtual string list_inactive_workers() = 0;
//	virtual string list_active_workers() = 0;
//	virtual string list_assigned_workers(Session_ptr) = 0;
//	virtual string list_sessions() = 0;
//	virtual std::map<Worker_ID, WorkerInfo> assign_workers(Session_ptr, uint16_t) = 0;

protected:
	string hostname;
	string address;
	uint16_t port;
	Session_ID next_session_ID;

	io_context & ic;
	Log_ptr log;

	std::map<Session_ID, Session_ptr> sessions;
	enum { max_recent_msgs = 100 };
	Message_queue recent_msgs;

	tcp::acceptor acceptor;

	void set_log(Log_ptr _log);

	void print_num_sessions();
//	void handshake(const Session_ptr session, Message & msg);
	int get_num_sessions();
	virtual void accept_connection() = 0;

	void print_IP();
};

typedef std::shared_ptr<Server> Server_ptr;

}

#endif
