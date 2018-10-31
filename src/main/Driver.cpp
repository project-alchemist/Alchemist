#include "Driver.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

Driver::Driver(io_context & _io_context, const unsigned int port) :
				Driver(_io_context, tcp::endpoint(tcp::v4(), port)) { }

Driver::Driver(io_context & _io_context, const tcp::endpoint & endpoint) :
				Server(_io_context, endpoint), next_matrix_ID(0), next_group_ID(0)
{
	log = start_log("driver", "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l]     %v");
	Server::set_log(log);

	lm = new LibraryManager(log);

	world = MPI_COMM_WORLD;

	int world_size;
	MPI_Comm_size(world, &world_size);

	num_workers = (uint16_t) world_size - 1;

	print_welcome_message();

	log->info("Starting workers");
	start_workers();

	log->info("Registering workers");
	register_workers();

	print_ready_message();

	std::thread t = std::thread(&Driver::accept_connection, this);

//	print_workers();
	t.join();
}

Driver::~Driver() { }

int Driver::start()
{
	accept_connection();

	return 0;
}

// ===============================================================================================
// ====================================   UTILITY FUNCTIONS   ====================================

// ----------------------------------------   Printing   -----------------------------------------

void Driver::print_welcome_message()
{
	string message = "=============================\n";
	message += SPACE;
	message += "Starting Alchemist {}\n";
	message += SPACE;
	message += "-----------------------------\n";
//	message += SPACE;
//	message += "Maximum number of OpenMP threads: {}\n";
	#ifndef ASIO_STANDALONE
	message += SPACE;
	message += "Using Boost.Asio {}\n";
	#endif
	message += SPACE;
	message += "Running on {} {}:{}";

	#ifndef ASIO_STANDALONE
	log->info(message.c_str(), get_Alchemist_version(), get_Boost_version(), hostname, address, port);
	#else
	log->info(message.c_str(), get_Alchemist_version(), hostname, address, port);
	#endif

//	int16_t i = 1;
//	int8_t *ptr;
//	ptr  = (int8_t*) &i;
//	(*ptr) ? std::cout << "little endian" << std::endl : std::cout << "big endian" << std::endl;
}

void Driver::print_ready_message()
{
	string message = "=============================\n";
	message += SPACE;
	message += "Alchemist is ready\n";
	message += SPACE;
	message += "-----------------------------";
	log->info(message.c_str());
}

//vector<Worker_ID> & Driver::allocate_workers(Session_ID session_ID, uint16_t num_workers)
//{
////	log->info("{} Allocating {} workers", sessions[session_ID].ptr->session_preamble(), num_workers);
//
//	vector<Worker_ID> allocated_workers(num_workers);
//
//	if (inactive_workers.size() >= num_workers) {
//		auto it = inactive_workers.begin();
//
//		Worker_ID worker_ID;
//
//		for (uint16_t i = 0; i < num_workers; i++) {
//			worker_ID = inactive_workers[i];
//			active_workers.insert(std::make_pair(worker_ID, session_ID));
//			workers[worker_ID].active = true;
////			allocated_workers[session_ID].push_back(worker_ID);
//		}
//		inactive_workers.erase(inactive_workers.begin(), inactive_workers.begin() + num_workers);
//	}
//
//	open_workers();
//
//	return allocated_workers[session_ID];
//}

void Driver::deallocate_workers(Session_ID session_ID)
{
//	for (Worker_ID worker_ID: allocated_workers[session_ID]) {
//		active_workers.erase(worker_ID);
//		workers[worker_ID].active = false;
//		inactive_workers.push_back(worker_ID);
//	}
//	allocated_workers[session_ID].clear();
}

string Driver::list_inactive_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of inactive workers:" << std::endl;

//	if (inactive_workers.size() == 0) {
//		list_of_workers << SPACE;
//		list_of_workers << "    No inactive workers" << std::endl;
//	}
//	else {
//		char buffer[4];
//
//		for (Worker_ID id = 1; id <= num_workers; ++id) {
//			if (!workers[id].active)
//			{
//				sprintf(buffer, "%03d", id);
//				list_of_workers << SPACE;
//				list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[id].hostname << " ";
//				list_of_workers << workers[id].address << ":" << workers[id].port << std::endl;
//			}
//		}
//	}

	return list_of_workers.str();
}

string Driver::list_active_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of active workers:" << std::endl;

//	if (active_workers.size() == 0) {
//		list_of_workers << SPACE;
//		list_of_workers << "    No active workers" << std::endl;
//	}
//	else {
//		char buffer[4];
//
//		for (Worker_ID id = 1; id <= num_workers; ++id) {
//			if (workers[id].active)
//			{
//				sprintf(buffer, "%03d", id);
//				list_of_workers << SPACE;
//				list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[id].hostname << " ";
//				list_of_workers << workers[id].address << ":" << workers[id].port << std::endl;
//			}
//		}
//	}

	return list_of_workers.str();
}

string Driver::list_allocated_workers(Client_ID client_ID)
{
	std::stringstream list_of_workers;
	list_of_workers << "List of assigned workers:" << std::endl;

//	if (allocated_workers[session_ID].size() == 0) {
//		list_of_workers << SPACE;
//		list_of_workers << "    No assigned workers" << std::endl;
//	}
//	else {
//		char buffer[4];
//
//		for (Worker_ID worker_ID: allocated_workers[session_ID]) {
//			sprintf(buffer, "%03d", worker_ID);
//			list_of_workers << SPACE;
//			list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[worker_ID].hostname << " ";
//			list_of_workers << workers[worker_ID].address << ":" << workers[worker_ID].port << std::endl;
//		}
//	}

	return list_of_workers.str();
}

// -----------------------------------------   Workers   -----------------------------------------


int Driver::start_workers()
{
	alchemist_command command = START;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
//	for (int temp_worker_ID = 1; temp_worker_ID <= num_workers; temp_worker_ID++) {
//		log->info("sfsfs {}", temp_worker_ID);
//		MPI_Isend(&command, 1, MPI_UNSIGNED_CHAR, temp_worker_ID, 0, world, &req);
//		MPI_Wait(&req, &status);
//	}
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(world);

	return 0;
}

int Driver::register_workers()
{
	alchemist_command command = SEND_INFO;

	log->info("Sending command {} to workers", get_command_name(command));

	Worker_ID temp_worker_ID;
	MPI_Request req;
	MPI_Status status;
//	for (int temp_worker_ID = 1; temp_worker_ID <= num_workers; temp_worker_ID++) {
//		MPI_Isend(&command, 1, MPI_UNSIGNED_CHAR, temp_worker_ID, 0, world, &req);
//		MPI_Wait(&req, &status);
//	}
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	uint16_t hostname_length, address_length, port;

	for (Worker_ID id = 1; id <= num_workers; ++id) {
		MPI_Recv(&hostname_length, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status);
		char hostname[hostname_length];
		MPI_Recv(hostname, hostname_length, MPI_CHAR, id, 0, world, &status);
		MPI_Recv(&address_length, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status);
		char address[address_length];
		MPI_Recv(address, address_length, MPI_CHAR, id, 0, world, &status);
		MPI_Recv(&port, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status);

		workers[id] = WorkerInfo();
		workers[id].hostname = string(hostname);
		workers[id].address = string(address);
		workers[id].port = port;

		unallocated_workers.push_back(id);
	}

	MPI_Barrier(world);

	if (num_workers == 0)
		log->info("No workers ready");
	else if (num_workers == 1)
		log->info("1 worker ready");
	else
		log->info("{} workers ready", num_workers);

	log->info(list_workers());

	return 0;
}

map<Worker_ID, WorkerInfo> Driver::allocate_workers(const Group_ID & group_ID, const uint16_t & num_requested_workers)
{
	map<Worker_ID, WorkerInfo> allocated_group;

	if (unallocated_workers.size() >= num_requested_workers) {

		Worker_ID worker_ID;

		for (uint16_t i = 0; i < num_requested_workers; i++) {
			worker_ID = unallocated_workers[i];
			allocated_workers.insert(std::make_pair(worker_ID, group_ID));
			workers[worker_ID].active = true;
			allocated_group.insert(std::make_pair(worker_ID, workers[worker_ID]));
		}
		unallocated_workers.erase(unallocated_workers.begin(), unallocated_workers.begin() + num_requested_workers);
	}

	open_workers(group_ID, allocated_group);

	log->info(list_workers());

	return allocated_group;
}

void Driver::open_workers(const Group_ID & group_ID, map<Worker_ID, WorkerInfo> & allocated_group)
{
	alchemist_command command = ACCEPT_CONNECTION;

	log->info("Sending command {} to workers", get_command_name(command));

	Worker_ID temp_worker_ID;
	MPI_Request req;
	MPI_Status status;
//	for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
//	for (int i = 1; i <= num_workers; i++) {
//		temp_worker_ID = i;
//		MPI_Isend(&command, 1, MPI_UNSIGNED_CHAR, temp_worker_ID, 0, world, &req);
//		MPI_Wait(&req, &status);
//	}
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	uint16_t group_size = allocated_group.size();
	uint16_t worker_ID, peer_ID;

	int group_IDs[group_size+1];
	group_IDs[0] = 0;

	int i = 0;
	for (int j = 1; j <= num_workers; j++) {
		worker_ID = j;
//  for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
		if (allocated_group.find(worker_ID) == allocated_group.end() ) {
			Group_ID null_group_ID = 0;
			MPI_Send(&null_group_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
		}
		else {
			group_IDs[i+1] = worker_ID;

			MPI_Send(&group_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
			MPI_Send(&group_size, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);

			for (auto it1 = allocated_group.begin(); it1 != allocated_group.end(); it1++) {
				peer_ID = it1->first;
				MPI_Send(&peer_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
			}
			i++;
		}
	}

	MPI_Group world_group;
	MPI_Group temp_group;

	MPI_Comm_group(world, &world_group);

	MPI_Group_incl(world_group, (int) (group_size+1), group_IDs, &temp_group);
	groups[group_ID]->set_group_comm(world, temp_group);
//	groups[group_ID]->say_something();
}

void Driver::idle_workers()
{
	alchemist_command command = IDLE;

	log->info("Sending command {} to workers", get_command_name(command));

	Worker_ID temp_worker_ID;
	MPI_Request req;
	MPI_Status status;
//	for (int temp_worker_ID = 1; temp_worker_ID <= num_workers; temp_worker_ID++) {
//		MPI_Isend(&command, 1, MPI_UNSIGNED_CHAR, temp_worker_ID, 0, world, &req);
//		MPI_Wait(&req, &status);
//	}
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);
}

string Driver::list_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of workers:" << std::endl;

	if (workers.size() == 0)
	{
		list_of_workers << SPACE;
		list_of_workers << "    No workers" << std::endl;
	}
	else
	{
		char buffer[4];

		for (Worker_ID id = 1; id <= num_workers; ++id) {
			sprintf(buffer, "%03d", id);
			list_of_workers << SPACE;
			list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[id].hostname << " at ";
			list_of_workers << workers[id].address << ":" << workers[id].port << " - ";
			if (workers[id].active) list_of_workers << "active";
			else list_of_workers << "idle";
			list_of_workers << std::endl;
		}
	}

	return list_of_workers.str();
}


int Driver::shut_down()
{
	log->info("Shutting down");

	return 0;
}

int Driver::send_assigned_workers_info()
{
//	log->info("    Sending worker hostnames and ports to Session {}");
//	auto num_workers = world.size() - 1;
//	output.write_int(num_workers);
//
//	for(auto id = 0; id < num_workers; ++id) {
//		output.write_string(workers[id].hostname);
//		output.write_int(workers[id].port);
//	}
//
//	output.flush();

	return 0;
}

int Driver::handle_command(int command) {


	return 0;
}

int Driver::run_task() {

//	string args = input.read_string();
//
//	MPI_Bcast(args.c_str(), args.length(), MPI_CHAR, 0, world);
//	boost::mpi::broadcast(world, args, 0);
//
//	Parameters output_parameters = Parameters();
//
//	int status = Executor::run_task(args, output_parameters);
//
//	log->info("    Output: {}", output_parameters.to_string());
//
//	if (status != 0) {
//		output.write_int(0x0);
//		return 1;
//	}
//
//	output.write_int(0x1);
//	output.write_string(output_parameters.to_string());

	return 0;
}

// ---------------------------------------   Information   ---------------------------------------


// -----------------------------------------   Library   -----------------------------------------

void Driver::print_num_sessions()
{
//	if (sessions.size() == 0)
//		log->info("No active sessions");
//	else if (sessions.size() == 1)
//		log->info("1 active session");
//	else
//		log->info("{} active sessions", sessions.size());
}

//void Driver::add_session(DriverSession_ptr session)
//{
//	Session_ID session_ID = session->get_ID();
//
//	sessions[session_ID] = session;
//
//	log->info("[Session {}] [{}:{}] Connection established", session->get_ID(), session->get_address().c_str(), session->get_port());
//	print_num_sessions();
//}
//
//void Driver::remove_session(Session_ID session_ID)
//{
//	Session_ID session_ID = session->get_ID();
//
//	log->info("Session {} at {} has been removed", sessions[session_ID]->get_ID(), sessions[session_ID]->get_address().c_str());
//	sessions.erase(session_ID);
//
//	print_num_sessions();
//}

int Driver::get_num_sessions()
{


//	return sessions.size();
	return 0;
}

// -----------------------------------------   Testing   -----------------------------------------



// ----------------------------------------   Matrices   -----------------------------------------


void Driver::new_group(tcp::socket socket)
{
	log->info("NEW GROUP");
	next_group_ID++;
	auto group_ptr = std::make_shared<GroupDriver>(next_group_ID, *this, log);
	groups.insert(std::make_pair(next_group_ID, group_ptr));
	groups[next_group_ID]->start(std::move(socket));
}

int Driver::accept_connection()
{
	if (groups.size() == 0) log->info("Accepting connections ...");
	acceptor.async_accept(
		[this](error_code ec, tcp::socket socket)
		{
//			if (!ec) std::make_shared<DriverSession>(std::move(socket), *this, next_session_ID++, log)->start();
			if (!ec) new_group(std::move(socket));

			accept_connection();
		});

	ic.run();

	return 0;
}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist



