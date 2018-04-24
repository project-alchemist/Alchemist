#include "Driver.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

Driver::Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint) :
    Executor(_world, _peers), Server(_io_context, endpoint), next_matrix_ID(1)
{
	log = start_log("driver");
	Executor::set_log(log);
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
}

Driver::~Driver() { }

int Driver::start()
{
	log->info("Accepting connections ...");
	accept_connection();

	return 0;
}

// ===============================================================================================
// ====================================   UTILITY FUNCTIONS   ====================================

// ----------------------------------------   Printing   -----------------------------------------

void Driver::print_welcome_message()
{
	string space("                                          ");
	string message = "Starting Alchemist\n";
	message += space;
	message += "-----------------------------\n";
	message += space;
	message += "Max number of OpenMP threads: {}\n";
	message += space;
	message += "Using Boost {}.{}.{}\n";
	message += space;
	message += "Running on {} {}:{}";

	log->info(message.c_str(), omp_get_max_threads(), BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100, hostname, address, port);
}

void Driver::print_ready_message()
{
	string message = "\n";
	message += "                                          =============================\n";
	message += "                                          Alchemist is ready\n";
	message += "                                          -----------------------------";
	log->info(message.c_str());
}

void Driver::print_inactive_workers()
{
	log->info("List of inactive workers:");

	if (inactive_workers.size() == 0)
		log->info("    No inactive workers");

	for(auto it = inactive_workers.begin(); it != inactive_workers.end(); ++it)
		log->info("    Worker {}: {}:{}", *it, workers[*it].hostname, workers[*it].port);
}

void Driver::print_active_workers()
{
	log->info("List of active workers:");

	if (active_workers.size() == 0)
		log->info("    No active workers");

	for(auto it = active_workers.begin(); it != active_workers.end(); ++it)
		log->info("    Worker {}: {}:{}, assigned to Session {}", it->first, workers[it->first].hostname, workers[it->first].port, it->second);
}

void Driver::print_sessions()
{
	log->info("List of current sessions:");

	if (sessions.size() == 0)
		log->info("    No active sessions");

//	for(auto it = sessions.begin(); it != sessions.end(); ++it)
//		log->info("    Session {}: {}:{}, assigned to Session {}", it->first, workers[it->first].hostname, workers[it->first].port, it->second);
}

string Driver::make_daytime_string()
{
	std::time_t now = std::time(0);
	return std::ctime(&now);
}

// -----------------------------------------   Workers   -----------------------------------------

int Driver::start_workers()
{

	unsigned char command_code = 0;

	MPI_Status status[1];
	uint16_t done;

	MPI_Bcast(&command_code, 1, MPI_UNSIGNED_CHAR, 0, world);

	for(uint16_t id = 1; id <= num_workers; ++id)
		MPI_Recv(&done, 1, MPI_UNSIGNED_CHAR, id, 0, world, &status[0]);

	return 0;
}

int Driver::register_workers()
{
	unsigned char command_code = 1;

	MPI_Bcast(&command_code, 1, MPI_UNSIGNED_CHAR, 0, world);

	MPI_Status status[3];
	uint16_t hostname_length, port;

	for(Worker_ID id = 1; id <= num_workers; ++id) {
		MPI_Recv(&hostname_length, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status[0]);
		char hostname[hostname_length];
		MPI_Recv(hostname, hostname_length, MPI_CHAR, id, 0, world, &status[1]);
		MPI_Recv(&port, 1, MPI_UNSIGNED_SHORT, id, 0, world, &status[2]);

		workers[id] = WorkerInfo();
		workers[id].hostname = string(hostname);
		workers[id].port = port;

		inactive_workers.push_back(id);
	}

	log->info("{} workers ready", num_workers);

	return 0;
}

// ----------------------------------------   File I/O   ----------------------------------------

int Driver::read_HDF5() {

	log->info("Driver::read_HDF5 not yet implemented");

	return 0;
}

// ===============================================================================================
// ====================================   COMMAND FUNCTIONS   ====================================

int Driver::handle_message(Session_ptr session, const Message & msg)
{
	unsigned char command_code = msg.command_code;

	bool should_exit = false;
	while (!should_exit) {

		log->info("Waiting for command");

		switch (command_code) {
			case 255:							// Shutdown
				log->info("Shutting down");
				shut_down();
				should_exit = true;
				break;
			case 1:
				receive_test_string(session, msg.body(), msg.body_length);
				break;
			case 2:
				send_test_string(session);
				break;
			case 3:
				assign_workers();
				break;
			case 4:
				send_assigned_worker_info();
				break;
			case 11:
				print_active_workers();
				break;
			case 12:
				print_inactive_workers();
				break;
			case 13:
				print_active_workers();
				break;
		}
	}

	return 0;
}

int Driver::receive_test_string(const Session_ptr session, const char * data, const uint32_t length) {

	string test_string(data, length);

	log->info("Test string from Session {} at {}", session->get_ID(), session->get_address());
	log->info(test_string);

	return 0;
}

int Driver::send_test_string(Session_ptr session) {

	session->write_string("This is a test string from Alchemist driver");

	return 0;
}

int Driver::shut_down()
{
	unload_libraries();

	return 0;
}

int Driver::assign_workers()
{


	send_assigned_worker_info();

	return 0;
}

int Driver::send_assigned_worker_info()
{
//	log->info("Sending worker hostnames and ports to Session {}");
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

int Driver::handle_command(int command_code) {


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
//	log->info("Output: {}", output_parameters.to_string());
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

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist



