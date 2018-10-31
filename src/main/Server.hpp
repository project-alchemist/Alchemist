#ifndef ALCHEMIST__SERVER_HPP
#define ALCHEMIST__SERVER_HPP

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Alchemist.hpp"
#include "Message.hpp"

namespace alchemist {

class Server// : public std::enable_shared_from_this<Server>
{
public:
	Server(io_context & _io_context, const tcp::endpoint & endpoint);
	Server(io_context & _io_context, const tcp::endpoint & endpoint, Log_ptr & _log);
	virtual ~Server() { }

//	void deliver(const Session_ptr session, Message & msg);


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

	io_context & ic;
	Log_ptr log;

	enum { max_recent_msgs = 100 };

	tcp::acceptor acceptor;

	void set_log(Log_ptr _log);

	virtual int accept_connection() = 0;

	void print_IP();
};

typedef std::shared_ptr<Server> Server_ptr;

}

#endif
