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

void Driver::set_group_communicator(const Group_ID & group_ID)
{
//	std::lock_guard<std::mutex> lock(worker_mutex);

	uint16_t group_size = (uint16_t) groups[group_ID]->workers.size();
	uint16_t worker_ID, peer_ID;

	int group_IDs[group_size+1];
	int group_peer_IDs[group_size];
	group_IDs[0] = 0;

	int i = 1;
	for (auto it = groups[group_ID]->workers.begin(); it != groups[group_ID]->workers.end(); it++) {
		log->info("Send {}", it->first);
		group_IDs[i] = it->first;
		group_peer_IDs[i-1] = it->first;
		i++;
	}

	MPI_Group world_group;
	MPI_Group temp_group;
	MPI_Group temp_peer_group;

	MPI_Comm_group(world, &world_group);

	MPI_Group_incl(world_group, (int) (group_size+1), group_IDs, &temp_group);
	MPI_Group_incl(world_group, (int) group_size, group_peer_IDs, &temp_peer_group);

	alchemist_command command = _AM_NEW_GROUP;

	log->info("Sending command {} to {} workers", get_command_name(command), group_size);
	for (int i = 0; i < group_size; i++) {
		log->info("S {}", group_IDs[i+1]);
	}
	groups[group_ID]->free_group();

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	uint16_t num_workers = (uint16_t) workers.size();

	for (int j = 1; j <= num_workers; j++) {
		worker_ID = j;
		if (groups[group_ID]->workers.find(worker_ID) == groups[group_ID]->workers.end() ) {
			Group_ID null_group_ID = 0;
			MPI_Send(&null_group_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
		}
		else {
			MPI_Send(&group_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
			MPI_Send(&group_size, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
			MPI_Send(&group_peer_IDs, (int) group_size, MPI_INT, worker_ID, 0, world);
		}
	}
	groups[group_ID]->set_group_comm(world, temp_group);

	groups[group_ID]->ready_group();

	MPI_Group_free(&world_group);
	MPI_Group_free(&temp_group);
	MPI_Group_free(&temp_peer_group);
//	MPI_Barrier(world);
}

// -----------------------------------------   Workers   -----------------------------------------


int Driver::start_workers()
{
	alchemist_command command = _AM_START;

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
	alchemist_command command = _AM_SEND_INFO;

	log->info("Sending command {} to workers", get_command_name(command));

	Worker_ID temp_worker_ID;
	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	uint16_t hostname_length, address_length, port;

	for (Worker_ID worker_ID = 1; worker_ID <= num_workers; ++worker_ID) {
		MPI_Recv(&hostname_length, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world, &status);
		char hostname[hostname_length];
		MPI_Recv(hostname, hostname_length, MPI_CHAR, worker_ID, 0, world, &status);
		MPI_Recv(&address_length, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world, &status);
		char address[address_length];
		MPI_Recv(address, address_length, MPI_CHAR, worker_ID, 0, world, &status);
		MPI_Recv(&port, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world, &status);

		workers.insert(std::make_pair(worker_ID, WorkerInfo(worker_ID, string(hostname), string(address), port)));

		unallocated_workers.push_back(worker_ID);
	}

	MPI_Barrier(world);

	if (num_workers == 0)
		log->info(string("No workers ready"));
	else if (num_workers == 1)
		log->info(string("1 worker ready"));
	else
		log->info("{} workers ready", num_workers);

	log->info(list_all_workers());

	return 0;
}

uint16_t Driver::allocate_workers(const Group_ID group_ID, const uint16_t & num_requested_workers)
{

	uint16_t num_allocated_workers = std::min(num_requested_workers, (uint16_t) unallocated_workers.size());

	std::lock_guard<std::mutex> lock(worker_mutex);

	log->info("zzzzzzzzzz {}", num_allocated_workers);

	if (num_allocated_workers > 0) {

		Worker_ID worker_ID;
		Group_ID _group_ID = group_ID;

		for (uint16_t i = 0; i < num_allocated_workers; i++) {
			worker_ID = unallocated_workers[i];
			workers.find(worker_ID)->second.active = true;
			workers.find(worker_ID)->second.group_ID = _group_ID;
			groups[group_ID]->add_worker(worker_ID, workers.find(worker_ID)->second);
		}
		unallocated_workers.erase(unallocated_workers.begin(), unallocated_workers.begin() + num_requested_workers);

		set_group_communicator(group_ID);
	}
	else
		log->info(string("No workers available to be allocated"));

	log->info(list_all_workers());

	return num_allocated_workers;
}

//void GroupDriver::new_group()
//{
//	alchemist_command command = _AM_NEW_GROUP;
//
//	//, map<Worker_ID, WorkerInfo> & allocated_group
//
//	log->info("Sending command {} to workers", get_command_name(command));
//
//	Worker_ID temp_worker_ID;
//	MPI_Request req;
//	MPI_Status status;
////	for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
////	for (int i = 1; i <= num_workers; i++) {
////		temp_worker_ID = i;
////		MPI_Isend(&command, 1, MPI_UNSIGNED_CHAR, temp_worker_ID, 0, world, &req);
////		MPI_Wait(&req, &status);
////	}
//	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
//	MPI_Wait(&req, &status);
//
//	uint16_t group_size = allocated_group.size();
//	uint16_t worker_ID, peer_ID;
//
//	int group_IDs[group_size+1];
//	group_IDs[0] = 0;
//
//	int i = 0;
//	for (int j = 1; j <= num_workers; j++) {
//		worker_ID = j;
////  for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
//		if (allocated_group.find(worker_ID) == allocated_group.end() ) {
//			Group_ID null_group_ID = 0;
//			MPI_Send(&null_group_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
//		}
//		else {
//			group_IDs[i+1] = worker_ID;
//
//			MPI_Send(&group_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
//			MPI_Send(&group_size, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
//
//			for (auto it1 = allocated_group.begin(); it1 != allocated_group.end(); it1++) {
//				peer_ID = it1->first;
//				MPI_Send(&peer_ID, 1, MPI_UNSIGNED_SHORT, worker_ID, 0, world);
//			}
//			i++;
//		}
//	}
//}

vector<Worker_ID> Driver::deallocate_workers(const Group_ID group_ID, const vector<Worker_ID> & selected_workers)
{
	std::lock_guard<std::mutex> lock(worker_mutex);

	vector<Worker_ID> deallocated_workers;

	for (auto it = selected_workers.begin(); it != selected_workers.end(); it++) {
		log->info("MMMMMMMMMMMMMMMMMMMMM {}", *it);
		unallocated_workers.push_back(*it);
		deallocated_workers.push_back(*it);
		workers.find(*it)->second.active = false;
		workers.find(*it)->second.group_ID = 0;
		groups[group_ID]->remove_worker(*it);
	}

	sort(unallocated_workers.begin(), unallocated_workers.end());

	set_group_communicator(group_ID);

	log->info(list_all_workers());

	return deallocated_workers;
}

string Driver::list_all_workers(const string & preamble)
{
	std::stringstream all_workers;
	auto num_workers = workers.size();

	string sp = SPACE;
	if (strcmp(preamble.c_str(), sp.c_str()) != 0) all_workers << preamble;

	if (num_workers == 0) {
		all_workers << "No workers";
		all_workers << std::endl;
	}
	else
	{
		char buffer[4];

		all_workers << "List of workers (" << num_workers << "):" << std::endl;

		for (auto it = workers.begin(); it != workers.end(); it++) {
			sprintf(buffer, "%03d", it->second.ID);
			all_workers << preamble;
			all_workers << "    Worker-" << string(buffer) << " running on " << it->second.hostname << " at ";
			all_workers << it->second.address << ":" << it->second.port << " - ";
			if (it->second.active) all_workers << "ACTIVE" << " (group " << it->second.group_ID << ")";
			else all_workers << "IDLE";
			all_workers << std::endl;
		}
	}

	return all_workers.str();
}

string Driver::list_all_workers()
{
	string space = SPACE;
	return Driver::list_all_workers(space);
}

string Driver::list_active_workers(const string & preamble)
{
	std::stringstream active_workers;

	auto num_active_workers = workers.size() - unallocated_workers.size();

	string sp = SPACE;
	if (strcmp(preamble.c_str(), sp.c_str()) != 0) active_workers << preamble;

	if (num_active_workers == 0) {
		active_workers << "No active workers";
		active_workers << std::endl;
	}
	else
	{
		char buffer[4];

		active_workers << "List of active workers (" << num_active_workers << "):" << std::endl;

		for (auto it = workers.begin(); it != workers.end(); it++) {
			if (it->second.active) {
				sprintf(buffer, "%03d", it->second.ID);
				active_workers << preamble;
				active_workers << "    Worker-" << string(buffer) << " running on " << it->second.hostname << " at ";
				active_workers << it->second.address << ":" << it->second.port << " (group " << it->second.group_ID << ")";
				active_workers << std::endl;
			}
		}
	}

	return active_workers.str();
}


string Driver::list_active_workers()
{
	string space = SPACE;
	return Driver::list_active_workers(space);
}

string Driver::list_inactive_workers(const string & preamble)
{
	std::stringstream inactive_workers;

	auto num_inactive_worker = unallocated_workers.size();

	string sp = SPACE;
	if (strcmp(preamble.c_str(), sp.c_str()) != 0) inactive_workers << preamble;

	if (num_inactive_worker == 0) {
		inactive_workers << "No inactive workers";
		inactive_workers << std::endl;
	}
	else
	{
		char buffer[4];

		inactive_workers << "List of inactive workers (" << num_inactive_worker << "):" << std::endl;

		for (auto it = unallocated_workers.begin(); it != unallocated_workers.end(); it++) {
			sprintf(buffer, "%03d", workers.find(*it)->second.ID);
			inactive_workers << preamble;
			inactive_workers << "    Worker-" << string(buffer) << " running on " << workers.find(*it)->second.hostname << " at ";
			inactive_workers << workers.find(*it)->second.address << ":" << workers.find(*it)->second.port;
			inactive_workers << std::endl;
		}
	}

	return inactive_workers.str();
}


string Driver::list_inactive_workers()
{
	string space = SPACE;
	return Driver::list_inactive_workers(space);
}

string Driver::list_allocated_workers(const Group_ID group_ID, const string & preamble)
{
	std::stringstream allocated_workers;
	auto num_group_workers = groups[group_ID]->workers.size();

	string sp = SPACE;
	if (strcmp(preamble.c_str(), sp.c_str()) != 0) allocated_workers << preamble;

	if (num_group_workers == 0) {
		allocated_workers << "No workers allocated to group " << group_ID;
		allocated_workers << std::endl;
	}
	else {
		char buffer[4];

		allocated_workers << "List of workers allocated to group " << group_ID << " (" << num_group_workers << "):" << std::endl;

		for (auto it = groups[group_ID]->workers.begin(); it != groups[group_ID]->workers.end(); it++) {
			sprintf(buffer, "%03d", it->second.ID);
			allocated_workers << preamble;
			allocated_workers << "    Worker-" << string(buffer) << " running on " << it->second.hostname << " at ";
			allocated_workers << it->second.address << ":" << it->second.port;
			allocated_workers << std::endl;
		}
	}

	return allocated_workers.str();
}

string Driver::list_allocated_workers(const Group_ID group_ID)
{
	string space = SPACE;
	return Driver::list_allocated_workers(group_ID, space);
}


uint16_t Driver::get_num_workers() {
	return num_workers;
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



