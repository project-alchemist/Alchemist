#include "Worker.hpp"

namespace alchemist {


Worker::Worker(MPI_Comm & _world, MPI_Comm & _peers, boost::asio::io_context & io_context, const tcp::endpoint & endpoint) :
    	    Executor(_world, _peers), Server(io_context, endpoint), ID(0) //, grid(El::mpi::Comm(_peers))
{
	int world_rank;
	MPI_Comm_rank(world, &world_rank);
	ID = world_rank;

	char buffer[12];
	sprintf(buffer, "worker-%03d", ID);

	log = start_log(string(buffer));
	Executor::set_log(log);
	Server::set_log(log);

	unsigned char temp;
	MPI_Bcast(&temp, 1, MPI_UNSIGNED_SHORT, 0, world);

	start();

	wait_for_command();
}

Worker::~Worker() { }

int Worker::start()
{
	char hostname[256];
	gethostname(hostname, sizeof(hostname));

	string space("                                              ");
	string message = "Worker ready\n";
	message += space;
	message += "Running on {} {}:{}\n";
	message += space;
	log->info(message.c_str(), string(hostname), acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

	char done = 1;

	MPI_Send(&done, 1, MPI_CHAR, 0, 0, world);

	return 0;
}

// ===============================================================================================
// ====================================   UTILITY FUNCTIONS   ====================================

// ----------------------------------------   File I/O   ----------------------------------------

int Worker::read_HDF5() {

	log->info("Worker::read_HDF5 not yet implemented");

	return 0;
}

// ===============================================================================================
// ====================================   COMMAND FUNCTIONS   ====================================

int Worker::handle_message(Session_ptr session, const Message & msg)
{
	unsigned char command_code = msg.command_code;

	bool should_exit = false;
	while (!should_exit) {

		log->info("Waiting for command");

		switch (command_code) {
		case 1:
			receive_test_string(session, msg.body(), msg.body_length);
			break;
		case 2:
			send_test_string(session);
			break;
		}
	}

	return 0;
}

int Worker::wait_for_command()
{
	unsigned char command_code;

	bool should_exit = false;
	while (!should_exit) {

		log->info("Waiting for command");

		MPI_Bcast(&command_code, 1, MPI_UNSIGNED_SHORT, 0, world);

		switch (command_code) {
			case 0:
				start();
				break;
			case 1:
				send_info();
				break;
		}
	}

	return 0;
}

int Worker::send_info() {

	log->info("Sending hostname and port to driver");

	uint16_t hl = hostname.length();

	MPI_Send(&hl, 1, MPI_UNSIGNED_SHORT, 0, 0, world);
	MPI_Send(hostname.c_str(), hl, MPI_CHAR, 0, 0, world);
	MPI_Send(&port, 1, MPI_UNSIGNED_SHORT, 0, 0, world);

	return 0;
}

int Worker::receive_test_string(const Session_ptr session, const char * data, const uint32_t length) {

	string test_string(data, length);

	log->info("Test string from Session {} at {}", session->get_ID(), session->get_address());
	log->info(test_string);

	return 0;
}

int Worker::send_test_string(Session_ptr session) {

	char buffer [100];
	sprintf (buffer, "This is a test string from Alchemist worker %d", ID);

	string test_string(buffer);
	session->write_string(test_string);

	return 0;
}



int Worker::run_task() {


	return 0;
}

// ----------------------------------------   Parameters   ---------------------------------------

int Worker::process_input_parameters(Parameters & input_parameters) {



	return 0;
}

int Worker::process_output_parameters(Parameters & output_parameters) {



	return 0;
}

// -----------------------------------------   Library   -----------------------------------------

int Worker::load_library() {



	return 0;
}

// -----------------------------------------   Testing   -----------------------------------------


// ----------------------------------------   Matrices   -----------------------------------------

MatrixHandle Worker::register_matrix(size_t num_rows, size_t num_cols) {

	MatrixHandle handle{0};


	return handle;
}

int Worker::receive_new_matrix() {



	return 0;
}

int Worker::get_matrix_dimensions() {


	return 0;
}

int Worker::get_transpose() {



	return 0;
}

int Worker::matrix_multiply() {



	return 0;
}

int Worker::get_matrix_rows() {



	return 0;
}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist
