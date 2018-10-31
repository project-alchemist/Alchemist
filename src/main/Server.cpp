#include "Server.hpp"

namespace alchemist {

Server::Server(io_context & _io_context, const tcp::endpoint & endpoint) :
		ic(_io_context), acceptor(_io_context, endpoint), log(nullptr)
{
	char _hostname[256];
	gethostname(_hostname, sizeof(_hostname));
	hostname = string(_hostname);

//	address = acceptor.local_endpoint().address().to_string();
	address = endpoint.address().to_string();
	port = endpoint.port();


//	int world_rank, world_size;
//	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
//	MPI_Comm_size(world, &world_size);
}

Server::Server(io_context & _io_context, const tcp::endpoint & endpoint, Log_ptr & _log) :
		ic(_io_context), acceptor(_io_context, endpoint), log(_log)
{
	char _hostname[256];
	gethostname(_hostname, sizeof(_hostname));
	hostname = string(_hostname);

//	address = acceptor.local_endpoint().address().to_string();
	address = endpoint.address().to_string();
	port = endpoint.port();


//	int world_rank, world_size;
//	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
//	MPI_Comm_size(world, &world_size);
}

void Server::set_log(Log_ptr _log)
{
	log = _log;
}


//void Server::deliver(const Session_ptr session, Message & msg)
//{
//
//
//	handle_message(session, msg);
//
////	int command_code = std::atoi(str.c_str());
//	//
//	//	handle_command(command_code);
//	//
//	//	log->info("Waiting on next command");
//}

void Server::print_IP()			// Outdated
{
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



