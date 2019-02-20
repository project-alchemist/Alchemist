#include "Driver.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

Driver::Driver(io_context & _io_context, const unsigned int port) :
				Driver(_io_context, tcp::endpoint(tcp::v4(), port)) { }

Driver::Driver(io_context & _io_context, const tcp::endpoint & endpoint) :
				Server(_io_context, endpoint), next_matrixID(0), next_groupID(0)
{
	log = start_log("driver", "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l]        %^%v%$", bold, iwhite);
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

//vector<WorkerID> & Driver::allocate_workers(SessionID sessionID, uint16_t num_workers)
//{
////	log->info("{} Allocating {} workers", sessions[sessionID].ptr->session_preamble(), num_workers);
//
//	vector<WorkerID> allocated_workers(num_workers);
//
//	if (inactive_workers.size() >= num_workers) {
//		auto it = inactive_workers.begin();
//
//		WorkerID workerID;
//
//		for (uint16_t i = 0; i < num_workers; i++) {
//			workerID = inactive_workers[i];
//			active_workers.insert(std::make_pair(workerID, sessionID));
//			workers[workerID].active = true;
////			allocated_workers[sessionID].push_back(workerID);
//		}
//		inactive_workers.erase(inactive_workers.begin(), inactive_workers.begin() + num_workers);
//	}
//
//	open_workers();
//
//	return allocated_workers[sessionID];
//}

void Driver::set_group_communicator(const GroupID & groupID)
{
//	std::lock_guard<std::mutex> lock(worker_mutex);

	uint16_t group_size = (uint16_t) groups[groupID]->workers.size();
	uint16_t workerID, peerID;

	int groupIDs[group_size+1];
	int group_peerIDs[group_size];
	groupIDs[0] = 0;

	int i = 1;
	for (auto it = groups[groupID]->workers.begin(); it != groups[groupID]->workers.end(); it++) {
		groupIDs[i] = it->first;
		group_peerIDs[i-1] = it->first;
		i++;
	}

	MPI_Group world_group;
	MPI_Group temp_group;
	MPI_Group temp_peer_group;

	MPI_Comm_group(world, &world_group);

	MPI_Group_incl(world_group, (int) (group_size+1), groupIDs, &temp_group);
	MPI_Group_incl(world_group, (int) group_size, group_peerIDs, &temp_peer_group);

	alchemist_command command = _AM_NEW_GROUP;

	log->info("Sending command {} to {} workers", get_command_name(command), group_size);
	groups[groupID]->free_group();

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	uint16_t num_workers = (uint16_t) workers.size();
	GroupID null_groupID = 0;

	int primary_group_worker = 0;

	for (int j = 1; j <= num_workers; j++) {
		workerID = j;
		if (groups[groupID]->workers.find(workerID) == groups[groupID]->workers.end() )
			MPI_Send(&null_groupID, 1, MPI_UNSIGNED_SHORT, workerID, 0, world);
		else {
			MPI_Send(&groupID, 1, MPI_UNSIGNED_SHORT, workerID, 0, world);
			MPI_Send(&group_size, 1, MPI_UNSIGNED_SHORT, workerID, 0, world);
			MPI_Send(&group_peerIDs, (int) group_size, MPI_INT, workerID, 0, world);
			MPI_Send(&primary_group_worker, 1, MPI_INT, workerID, 0, world);
			if (primary_group_worker == 0) primary_group_worker = 1;
		}
	}
	groups[groupID]->set_group_comm(world, temp_group);

	groups[groupID]->ready_group();

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
//	for (int temp_workerID = 1; temp_workerID <= num_workers; temp_workerID++) {
//		log->info("sfsfs {}", temp_workerID);
//		MPI_Isend(&command, 1, MPI_UNSIGNED_CHAR, temp_workerID, 0, world, &req);
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

	WorkerID temp_workerID;
	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	uint16_t hostname_length, address_length, port;

	for (WorkerID workerID = 1; workerID <= num_workers; ++workerID) {
		MPI_Recv(&hostname_length, 1, MPI_UNSIGNED_SHORT, workerID, 0, world, &status);
		char hostname[hostname_length];
		MPI_Recv(hostname, hostname_length, MPI_CHAR, workerID, 0, world, &status);
		MPI_Recv(&address_length, 1, MPI_UNSIGNED_SHORT, workerID, 0, world, &status);
		char address[address_length];
		MPI_Recv(address, address_length, MPI_CHAR, workerID, 0, world, &status);
		MPI_Recv(&port, 1, MPI_UNSIGNED_SHORT, workerID, 0, world, &status);

		workers.insert(std::make_pair(workerID, std::make_shared<WorkerInfo>(workerID, string(hostname), string(address), port)));

		unallocated_workers.push_back(workerID);
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

uint16_t Driver::allocate_workers(const GroupID groupID, const uint16_t & num_requested_workers)
{
	uint16_t num_allocated_workers = std::min(num_requested_workers, (uint16_t) unallocated_workers.size());

	std::lock_guard<std::mutex> lock(worker_mutex);

	if (num_allocated_workers > 0) {

		WorkerID workerID;
		GroupID _groupID = groupID;

		for (uint16_t i = 0; i < num_allocated_workers; i++) {
			workerID = unallocated_workers[i];
			workers.find(workerID)->second->groupID = _groupID;
			groups[groupID]->add_worker(workerID, workers.find(workerID)->second);
		}
		unallocated_workers.erase(unallocated_workers.begin(), unallocated_workers.begin() + num_requested_workers);

		set_group_communicator(groupID);
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
//	//, map<WorkerID, WorkerInfo> & allocated_group
//
//	log->info("Sending command {} to workers", get_command_name(command));
//
//	WorkerID temp_workerID;
//	MPI_Request req;
//	MPI_Status status;
////	for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
////	for (int i = 1; i <= num_workers; i++) {
////		temp_workerID = i;
////		MPI_Isend(&command, 1, MPI_UNSIGNED_CHAR, temp_workerID, 0, world, &req);
////		MPI_Wait(&req, &status);
////	}
//	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
//	MPI_Wait(&req, &status);
//
//	uint16_t group_size = allocated_group.size();
//	uint16_t workerID, peerID;
//
//	int groupIDs[group_size+1];
//	groupIDs[0] = 0;
//
//	int i = 0;
//	for (int j = 1; j <= num_workers; j++) {
//		workerID = j;
////  for (auto it = allocated_group.begin(); it != allocated_group.end(); it++) {
//		if (allocated_group.find(workerID) == allocated_group.end() ) {
//			GroupID null_groupID = 0;
//			MPI_Send(&null_groupID, 1, MPI_UNSIGNED_SHORT, workerID, 0, world);
//		}
//		else {
//			groupIDs[i+1] = workerID;
//
//			MPI_Send(&groupID, 1, MPI_UNSIGNED_SHORT, workerID, 0, world);
//			MPI_Send(&group_size, 1, MPI_UNSIGNED_SHORT, workerID, 0, world);
//
//			for (auto it1 = allocated_group.begin(); it1 != allocated_group.end(); it1++) {
//				peerID = it1->first;
//				MPI_Send(&peerID, 1, MPI_UNSIGNED_SHORT, workerID, 0, world);
//			}
//			i++;
//		}
//	}
//}

vector<WorkerID> Driver::deallocate_workers(const GroupID groupID, const vector<WorkerID> & selected_workers)
{
	std::lock_guard<std::mutex> lock(worker_mutex);

	vector<WorkerID> deallocated_workers;

	for (auto it = selected_workers.begin(); it != selected_workers.end(); it++) {
		unallocated_workers.push_back(*it);
		deallocated_workers.push_back(*it);
		workers.find(*it)->second->groupID = 0;
		groups[groupID]->remove_worker(*it);
	}

	sort(unallocated_workers.begin(), unallocated_workers.end());

	set_group_communicator(groupID);

	log->info(list_all_workers());

	return deallocated_workers;
}

vector<WorkerInfo_ptr> Driver::get_all_workers()
{
	vector<WorkerInfo_ptr> all_workers;

	for (auto it = workers.begin(); it != workers.end(); it++)
		all_workers.push_back(it->second);

	return all_workers;
}

vector<WorkerInfo_ptr> Driver::get_active_workers()
{
	vector<WorkerInfo_ptr> active_workers;

	for (auto it = workers.begin(); it != workers.end(); it++)
		if (it->second->groupID > 0) active_workers.push_back(it->second);

	return active_workers;
}

vector<WorkerInfo_ptr> Driver::get_inactive_workers()
{
	vector<WorkerInfo_ptr> inactive_workers;

	for (auto it = workers.begin(); it != workers.end(); it++)
		if (it->second->groupID == 0) inactive_workers.push_back(it->second);

	return inactive_workers;
}

vector<WorkerInfo_ptr> Driver::get_assigned_workers(const GroupID groupID)
{
	vector<WorkerInfo_ptr> assigned_workers;

	for (auto it = workers.begin(); it != workers.end(); it++)
		if (it->second->groupID == groupID) assigned_workers.push_back(it->second);

	return assigned_workers;
}

string Driver::list_all_workers(const string & preamble)
{
	std::stringstream all_workers;
	auto num_workers = workers.size();

	string sp = SPACE;
	if (strcmp(preamble.c_str(), sp.c_str()) != 0) all_workers << preamble;

	if (num_workers == 0)
		all_workers << "No workers" << std::endl;
	else
	{
		all_workers << "List of workers (" << num_workers << "):" << std::endl;

		for (auto it = workers.begin(); it != workers.end(); it++)
			all_workers << preamble << "    " << it->second->to_string() << std::endl;
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

	if (num_active_workers == 0)
		active_workers << "No active workers" << std::endl;
	else
	{
		active_workers << "List of active workers (" << num_active_workers << "):" << std::endl;

		for (auto it = workers.begin(); it != workers.end(); it++)
			if (it->second->groupID > 0)
				active_workers << preamble << "    " << it->second->to_string() << std::endl;
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

	if (num_inactive_worker == 0)
		inactive_workers << "No inactive workers" << std::endl;
	else
	{
		inactive_workers << "List of inactive workers (" << num_inactive_worker << "):" << std::endl;

		for (auto it = workers.begin(); it != workers.end(); it++)
			if (it->second->groupID == 0)
				inactive_workers << preamble << "    " << it->second->to_string(false) << std::endl;
	}

	return inactive_workers.str();
}


string Driver::list_inactive_workers()
{
	string space = SPACE;
	return Driver::list_inactive_workers(space);
}

string Driver::list_allocated_workers(const GroupID groupID, const string & preamble)
{
	std::stringstream allocated_workers;
	auto num_group_workers = groups[groupID]->workers.size();

	string sp = SPACE;
	if (strcmp(preamble.c_str(), sp.c_str()) != 0) allocated_workers << preamble;

	if (num_group_workers == 0)
		allocated_workers << "No workers allocated to group " << groupID << std::endl;
	else {
		allocated_workers << "List of workers allocated to group " << groupID << " (" << num_group_workers << "):" << std::endl;

		for (auto it = workers.begin(); it != workers.end(); it++)
			if (it->second->groupID == groupID)
				allocated_workers << preamble << "    " << it->second->to_string(false) << std::endl;
	}

	return allocated_workers.str();
}

string Driver::list_allocated_workers(const GroupID groupID)
{
	string space = SPACE;
	return Driver::list_allocated_workers(groupID, space);
}


uint16_t Driver::get_num_workers()
{
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
//	SessionID sessionID = session->getID();
//
//	sessions[sessionID] = session;
//
//	log->info("[Session {}] [{}:{}] Connection established", session->getID(), session->get_address().c_str(), session->get_port());
//	print_num_sessions();
//}
//
//void Driver::remove_session(SessionID sessionID)
//{
//	SessionID sessionID = session->getID();
//
//	log->info("Session {} at {} has been removed", sessions[sessionID]->getID(), sessions[sessionID]->get_address().c_str());
//	sessions.erase(sessionID);
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
	next_groupID++;
	auto group_ptr = std::make_shared<GroupDriver>(next_groupID, *this, log);
	groups.insert(std::make_pair(next_groupID, group_ptr));
	groups[next_groupID]->start(std::move(socket));
}


int Driver::accept_connection()
{
	if (groups.size() == 0) log->info("Accepting connections ...");
	acceptor.async_accept(
		[this](error_code ec, tcp::socket socket)
		{
//			if (!ec) std::make_shared<DriverSession>(std::move(socket), *this, next_sessionID++, log)->start();
			if (!ec) new_group(std::move(socket));

			accept_connection();
		});

	ic.run();

	return 0;
}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist



