#include "Worker.hpp"

namespace alchemist {

Worker::Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port) :
    	    Worker(_world, _peers, _io_context, tcp::endpoint(tcp::v4(), port)) { }

Worker::Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint) :
			world(_world), peers(_peers), Server(_io_context, endpoint), ID(0)
{
	int world_rank;
	MPI_Comm_rank(world, &world_rank);
	ID = world_rank;

	char buffer[12];
	sprintf(buffer, "worker-%03d", ID);

	log = start_log(string(buffer));
//	Executor::set_log(log);
	Server::set_log(log);

	wait_for_command();
}

Worker::~Worker() { }

int Worker::start()
{
	char hostname[256];
	gethostname(hostname, sizeof(hostname));

	string message = "Worker ready\n";
	message += SPACE;
	message += "Running on {} {}:{}";
	log->info(message.c_str(), string(hostname), acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

	MPI_Barrier(world);

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

int Worker::handle_message(WorkerSession_ptr session, Message & msg)
{
	client_command c = msg.cc;

	bool should_exit = false;
	while (!should_exit) {

		log->info("Waiting for command");

		switch (c) {
//		case RECEIVE_TEST_STRING:
//			receive_test_string(session, msg.body(), msg.body_length);
//			break;
		case SEND_TEST_STRING:
			send_test_string(session);
			break;
		}
	}

	return 0;
}

int Worker::wait_for_command()
{
	alchemist_command c;

	bool should_exit = false;
	int flag = 0;
	MPI_Request req = MPI_REQUEST_NULL;
	MPI_Status status;

	while (!should_exit) {

		MPI_Ibcast(&c, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
		while (flag == 0) {
			MPI_Test(&req, &flag, &status);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		switch (c) {
			case IDLE:
				break;
			case START:
				start();
				break;
			case SEND_INFO:
				send_info();
				break;
			case ACCEPT_CONNECTION:
				accept_connection();

				ic.run();
				break;
		}

		flag = 0;
		c = IDLE;
	}

	return 0;
}

int Worker::send_info() {

	log->info("Sending hostname and port to driver");

	std::string address = acceptor.local_endpoint().address().to_string();

	uint16_t hl = hostname.length()+1;
	uint16_t al = address.length()+1;

	MPI_Send(&hl, 1, MPI_UNSIGNED_SHORT, 0, 0, world);
	MPI_Send(hostname.c_str(), hl, MPI_CHAR, 0, 0, world);
	MPI_Send(&al, 1, MPI_UNSIGNED_SHORT, 0, 0, world);
	MPI_Send(address.c_str(), al, MPI_CHAR, 0, 0, world);
	MPI_Send(&port, 1, MPI_UNSIGNED_SHORT, 0, 0, world);

	return 0;
}

int Worker::receive_test_string(const WorkerSession_ptr session, const char * data, const uint32_t length) {

	string test_string(data, length);

	log->info("Test string from Session {} at {}", session->get_ID(), session->get_address());
	log->info(test_string);

	return 0;
}

int Worker::send_test_string(WorkerSession_ptr session) {

	char buffer [100];
	sprintf (buffer, "This is a test string from Alchemist worker %d", ID);

	string test_string(buffer);
	session->write_string(test_string);

	return 0;
}


string Worker::list_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of workers:" << std::endl;

	return list_of_workers.str();

}

string Worker::list_active_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of workers:" << std::endl;

	return list_of_workers.str();

}

string Worker::list_inactive_workers()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of workers:" << std::endl;

	return list_of_workers.str();

}

string Worker::list_sessions()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of session:" << std::endl;

	return list_of_workers.str();

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

//MatrixHandle Worker::register_matrix(size_t num_rows, size_t num_cols) {
//
//	MatrixHandle handle{0};
//
//
//	return handle;
//}

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


void Worker::accept_connection()
{
	log->info("Accepting connections ...");
	acceptor.async_accept(
		[this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec) {
				std::make_shared<WorkerSession>(std::move(socket), *this, next_session_ID++, log)->start();
			}

			accept_connection();
		});
}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist
