#include "Driver.hpp"

namespace alchemist {

const string get_Alchemist_version()
{
	std::stringstream ss;
	ss << ALCHEMIST_VERSION_MAJOR << "." << ALCHEMIST_VERSION_MINOR;
	return ss.str();
}

const string get_Boost_version()
{
	std::stringstream ss;
	ss << BOOST_VERSION / 100000 << "." << BOOST_VERSION / 100 % 1000 << "." << BOOST_VERSION % 100;
	return ss.str();
}

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

Driver::Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port) :
		Driver(_world, _peers, _io_context, tcp::endpoint(tcp::v4(), port)) { }

Driver::Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint) :
		world(_world), peers(_peers), Server(_io_context, endpoint), next_matrix_ID(1)
{
	log = start_log("driver");
	log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l]     %v");
//	Executor::set_log(log);
	Server::set_log(log);

	int world_size;
	MPI_Comm_size(world, &world_size);

	num_workers = world_size - 1;

	print_welcome_message();

	log->info("Starting workers");
	start_workers();

	log->info("Registering workers");
	register_workers();

	print_ready_message();

	start();
//	MPI_Status status[1];
//	uint16_t done;
//
//
//	for(uint16_t id = 1; id <= num_workers; ++id)
//		MPI_Recv(&done, 1, MPI_UNSIGNED_CHAR, id, 0, world, &status[0]);

}



Driver::~Driver() { }

int Driver::start()
{
	open_workers();
	accept_connection();

	ic.run();


//	alchemist_command command = ACCEPT_CONNECTION;
//	MPI_Bcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world);

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
	message += SPACE;
	message += "Maximum number of OpenMP threads: {}\n";
	message += SPACE;
	message += "Using Boost.Asio {}\n";
	message += SPACE;
	message += "Running on {} {}:{}";

	log->info(message.c_str(), get_Alchemist_version(), omp_get_max_threads(), get_Boost_version(), hostname, address, port);
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

std::map<Worker_ID, WorkerInfo> Driver::allocate_workers(DriverSession_ptr s, uint16_t num_workers)
{
	Session_ID session_ID = s->get_ID();

	if (inactive_workers.size() >= num_workers) {

		auto it = inactive_workers.begin();

		for (uint16_t i = 0; i < num_workers; i++) {
			uint16_t id = inactive_workers[i];
			active_workers.insert(std::make_pair(id, session_ID));
			workers[id].active = true;
			allocated_workers[session_ID].insert(std::make_pair(id, workers[id]));
		}
		inactive_workers.erase(inactive_workers.begin(), inactive_workers.begin() + num_workers);
	}

	return allocated_workers[session_ID];
}

void Driver::deallocate_workers(DriverSession_ptr s)
{
	Session_ID session_ID = s->get_ID();

	for (auto it = allocated_workers[session_ID].begin() ; it != allocated_workers[session_ID].end(); it++) {
		uint16_t id = it->first;
		active_workers.erase(id);
		workers[id].active = false;
		inactive_workers.push_back(id);
	}
	allocated_workers[session_ID].clear();
}

string Driver::list_inactive_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of inactive workers:" << std::endl;

	if (inactive_workers.size() == 0) {
		list_of_workers << SPACE;
		list_of_workers << "    No inactive workers" << std::endl;
	}
	else {
		char buffer[4];

		for (Worker_ID id = 1; id <= num_workers; ++id) {
			if (!workers[id].active)
			{
				sprintf(buffer, "%03d", id);
				list_of_workers << SPACE;
				list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[id].hostname << " ";
				list_of_workers << workers[id].address << ":" << workers[id].port << std::endl;
			}
		}
	}

	return list_of_workers.str();
}

string Driver::list_active_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of active workers:" << std::endl;

	if (active_workers.size() == 0) {
		list_of_workers << SPACE;
		list_of_workers << "    No active workers" << std::endl;
	}
	else {
		char buffer[4];

		for (Worker_ID id = 1; id <= num_workers; ++id) {
			if (workers[id].active)
			{
				sprintf(buffer, "%03d", id);
				list_of_workers << SPACE;
				list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[id].hostname << " ";
				list_of_workers << workers[id].address << ":" << workers[id].port << std::endl;
			}
		}
	}

	return list_of_workers.str();
}

string Driver::list_allocated_workers(DriverSession_ptr s)
{
	Session_ID session_ID = s->get_ID();
	std::stringstream list_of_workers;
	list_of_workers << "List of assigned workers:" << std::endl;

	if (allocated_workers[session_ID].size() == 0) {
		list_of_workers << SPACE;
		list_of_workers << "    No assigned workers" << std::endl;
	}
	else {
		char buffer[4];

		for (auto const& worker: allocated_workers[session_ID]) {
			sprintf(buffer, "%03d", worker.first);
			list_of_workers << SPACE;
			list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[worker.first].hostname << " ";
			list_of_workers << workers[worker.first].address << ":" << workers[worker.first].port << std::endl;
		}
	}

	return list_of_workers.str();
}

string Driver::list_sessions()
{
	std::stringstream list_of_sessions;
	list_of_sessions << "List of current sessions:" << std::endl;
	log->info("List of current sessions:");

	if (workers.size() == 0)
		list_of_sessions << "    No active sessions" << std::endl;

	return list_of_sessions.str();

//	for(auto it = sessions.begin(); it != sessions.end(); ++it)
//		log->info("        Session {}: {}:{}, assigned to Session {}", it->first, workers[it->first].hostname, workers[it->first].port, it->second);
}

string Driver::make_daytime_string()
{
	std::time_t now = std::time(0);
	return std::ctime(&now);
}

// -----------------------------------------   Workers   -----------------------------------------

int Driver::start_workers()
{
	alchemist_command command = START;

//	log->info("Sending command {}", command);

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(world);

	return 0;
}

int Driver::register_workers()
{
	alchemist_command command = SEND_INFO;

//	log->info("Sending command {}", command);
	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
	MPI_Wait(&req, &status);

	uint16_t hostname_length, address_length, port;

	for (Worker_ID id = 1; id <= num_workers; ++id) {
		MPI_Recv(&hostname_length, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status);
		char hostname[hostname_length];
		MPI_Recv(hostname, hostname_length, MPI_CHAR, id, 0, world, &status);
		MPI_Recv(&address_length, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status);
		char address[hostname_length];
		MPI_Recv(address, address_length, MPI_CHAR, id, 0, world, &status);
		MPI_Recv(&port, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status);

		workers[id] = WorkerInfo();
		workers[id].hostname = string(hostname);
		workers[id].address = string(address);
		workers[id].port = port;

		inactive_workers.push_back(id);
	}

	if (num_workers == 0)
		log->info("No workers ready");
	else if (num_workers == 1)
		log->info("1 worker ready:");
	else
		log->info("{} workers ready:", num_workers);

	log->info(list_workers());

	return 0;
}

void Driver::open_workers()
{
	alchemist_command command = ACCEPT_CONNECTION;

//	log->info("Sending command {}", command);

	MPI_Request req;
	MPI_Status status;
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
			list_of_workers << "    Worker-" << string(buffer) << " running on " << workers[id].hostname << " ";
			list_of_workers << workers[id].address << ":" << workers[id].port << " - ";
			if (workers[id].active) list_of_workers << "active";
			else list_of_workers << "idle";
			list_of_workers << std::endl;
		}
	}

	return list_of_workers.str();
}

// ----------------------------------------   File I/O   ----------------------------------------

int Driver::read_HDF5() {

	log->info("Driver::read_HDF5 not yet implemented");



	return 0;
}

// ===============================================================================================
// ====================================   COMMAND FUNCTIONS   ====================================

int Driver::receive_test_string(const DriverSession_ptr session, const char * data, const uint32_t length) {

	string test_string(data, length);

	log->info("[Session {}] [{}] Received test string from client", session->get_ID(), session->get_address());
	log->info(test_string);

	return 0;
}

int Driver::send_test_string(DriverSession_ptr session) {

	session->write_string("This is a test string from Alchemist driver");

	return 0;
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


// ----------------------------------------   Parameters   ---------------------------------------


int Driver::process_input_parameters(Parameters & input_parameters) {


	return 0;
}


int Driver::process_output_parameters(Parameters & output_parameters) {



	return 0;
}

// -----------------------------------------   Library   -----------------------------------------

int Driver::load_library() {



	return 0;
}

// -----------------------------------------   Testing   -----------------------------------------



// ----------------------------------------   Matrices   -----------------------------------------


//MatrixHandle Driver::register_matrix(size_t num_rows, size_t num_cols) {
//
//	MatrixHandle handle{next_matrix_ID++};
//
//	return handle;
//}


int Driver::receive_new_matrix() {


	return 0;
}


int Driver::get_matrix_dimensions() {


	return 0;
}


int Driver::get_transpose() {


	return 0;
}


int Driver::matrix_multiply() {


	return 0;
}


int Driver::get_matrix_rows() {



	return 0;
}



void Driver::accept_connection()
{
	log->info("Accepting connections ...");
	acceptor.async_accept(
		[this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec) {
				std::make_shared<DriverSession>(std::move(socket), *this, next_session_ID++, log)->start();
			}

			accept_connection();
		});
}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist



