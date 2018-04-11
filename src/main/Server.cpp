#include "Server.hpp"

namespace alchemist {

Server::Server(boost::asio::io_context & io_context, const tcp::endpoint & endpoint) : acceptor(io_context, endpoint), next_session_ID(1)
{
	char _hostname[256];
	gethostname(_hostname, sizeof(_hostname));

	hostname = string(_hostname);

	address = endpoint.address().to_string();
	port = endpoint.port();
}

void Server::set_log(Log_ptr _log)
{
	log = _log;
}

void Server::print_num_sessions()
{
	if (sessions.size() == 0)
		log->info("No active sessions");
	else if (sessions.size() == 1)
		log->info("1 active session");
	else
		log->info("{} active sessions", sessions.size());
}

void Server::add_session(Session_ptr session)
{
	Session_ID session_ID = session->get_ID();

	sessions[session_ID] = session;

	log->info("Session {} at {} has been added", session->get_ID(), session->get_address().c_str());
	print_num_sessions();
}

void Server::remove_session(Session_ptr session)
{
	Session_ID session_ID = session->get_ID();

	log->info("Session {} at {} has been removed", sessions[session_ID]->get_ID(), sessions[session_ID]->get_address().c_str());
	sessions.erase(session_ID);

	print_num_sessions();
}

int Server::get_num_sessions()
{
	return sessions.size();
}

void Server::deliver(Session_ptr session, const Message & msg)
{

	log->info("Received message from Session {} at {}: {}", session->get_ID(), session->get_address().c_str(), msg.body());

	handle_message(session, msg);

//	int command_code = std::atoi(str.c_str());
	//
	//	handle_command(command_code);
	//
	//	log->info("Waiting on next command");
}

int Server::handshake() {


	return 0;
}

void Server::accept_connection()
{
	acceptor.async_accept(
		[this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec) {
				std::make_shared<Session>(std::move(socket), *this, next_session_ID++)->start();
			}

			accept_connection();
		});
}

void Server::print_IP()
{
	// Outdated
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP address

	if ((rv = getaddrinfo(NULL, "24960", &hints, &servinfo)) != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		char hostname[NI_MAXHOST];
		getnameinfo(p->ai_addr, p->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0);
		std::string ip(hostname);
		log->info("IP: {}", hostname);
	}

	freeaddrinfo(servinfo); // all done with this structure
}

// ===============================================================================================
// ===============================================================================================


} // namespace alchemist



