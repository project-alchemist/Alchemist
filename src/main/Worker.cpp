#include "Worker.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

//
//Worker::Worker(io_context & _io_context, tcp::endpoint & _endpoint) :
//				ic(_io_context), endpoint(_endpoint), ID(0), client_ID(0), next_session_ID(0), accept_connections(false)

Worker::Worker(io_context & _io_context, const unsigned int _port) :
		ic(_io_context), group_worker(nullptr), ID(0), client_ID(0), next_session_ID(0), accept_connections(false)
{
	world = MPI_COMM_WORLD;

	endpoint = tcp::endpoint(tcp::v4(), _port);

	char _hostname[256];
	gethostname(_hostname, sizeof(_hostname));
	hostname = string(_hostname);

//	address = acceptor.local_endpoint().address().to_string();
	address = endpoint.address().to_string();
	port = endpoint.port();

	int world_rank;
	MPI_Comm_rank(world, &world_rank);
	ID = (uint16_t) world_rank;

	char buffer[12];
	sprintf(buffer, "worker-%03d", ID);

	log = start_log(string(buffer), "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");

	wait_for_command();
}

Worker::~Worker() { }


//Worker_ID Worker::get_ID()
//{
//	return ID;
//}
//
//bool Worker::is_active()
//{
//	return (sessions.size() == 0);
//}
//

// ===============================================================================================
// ====================================   COMMAND FUNCTIONS   ====================================

int Worker::wait_for_command()
{
//	log->info("DEBUG: Worker: wait_for_command");

	alchemist_command c;

	bool should_exit = false;
	int flag = 0;
	MPI_Request req = MPI_REQUEST_NULL;
	MPI_Status status;

	while (!should_exit) {

//		MPI_Irecv(&c, 1, MPI_UNSIGNED_CHAR, 0, 0, world, &req);
		MPI_Ibcast(&c, 1, MPI_UNSIGNED_CHAR, 0, world, &req);
		while (flag == 0) {
			MPI_Test(&req, &flag, &status);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		threads.push_back(std::thread(&Worker::handle_command, this, c));

		flag = 0;
		c = _AM_IDLE;

//		log->info("Number of threads: {}", threads.size());
	}

	for (auto & t: threads) t.join();

	return 0;
}


Worker_ID Worker::get_ID()
{
	return ID;
}

int Worker::handle_command(alchemist_command c)
{
//	log->info("DEBUG: Worker: handle_command {}", c);

	switch (c) {
		case _AM_IDLE:
			break;
		case _AM_START:
			start();
			break;
		case _AM_SEND_INFO:
			send_info();
			break;
		case _AM_NEW_GROUP:
			handle_new_group();
			break;
		case _AM_NEW_SESSION:
//			new_session();
			break;
//		case END_SESSION:
//			end_session();
//			break;
//		case _AM_ACCEPT_CONNECTION:
//			get_group_peers();
//			break;
//		case NEW_MATRIX:
//			new_matrix();
//			break;
//		case CLIENT_MATRIX_LAYOUT:
//			get_matrix_layout();
//			break;
	}

	return 0;
}

int Worker::start()
{
	string message = "Worker ready\n";
	message += SPACE;
	message += "Running on {} {}:{}";
	log->info(message.c_str(), hostname, address, port);

	MPI_Barrier(world);

	return 0;
}

void Worker::send_info()
{
	log->info("Sending hostname and port to driver");

	uint16_t hl = hostname.length()+1;
	uint16_t al = address.length()+1;

	MPI_Send(&hl, 1, MPI_UNSIGNED_SHORT, 0, 0, world);
	MPI_Send(hostname.c_str(), hl, MPI_CHAR, 0, 0, world);
	MPI_Send(&al, 1, MPI_UNSIGNED_SHORT, 0, 0, world);
	MPI_Send(address.c_str(), al, MPI_CHAR, 0, 0, world);
	MPI_Send(&port, 1, MPI_UNSIGNED_SHORT, 0, 0, world);

	MPI_Barrier(world);
}

void Worker::handle_new_group()
{
	MPI_Status status;

	Group_ID group_ID;

	MPI_Recv(&group_ID, 1, MPI_UNSIGNED_SHORT, 0, 0, world, &status);
	if (group_ID > 0) {

		uint16_t num_peers = 0;

		MPI_Recv(&num_peers, 1, MPI_UNSIGNED_SHORT, 0, 0, world, &status);

		int group_IDs[(int) num_peers+1];
		int group_peer_IDs[(int) num_peers];

		group_IDs[0] = 0;
		MPI_Recv(&group_peer_IDs, (int) num_peers, MPI_INT, 0, 0, world, &status);
		for (int i = 0; i < num_peers; i++) group_IDs[i+1] = group_peer_IDs[i];

		if (group_worker == nullptr)
			group_worker = std::make_shared<GroupWorker>(group_ID, *this, ic, port, log);

		MPI_Group world_group;
		MPI_Group temp_group;
		MPI_Comm_group(world, &world_group);

		MPI_Group_incl(world_group, (int) (num_peers+1), group_IDs, &temp_group);
		group_worker->set_group_comm(world, temp_group);

		MPI_Group_incl(world_group, (int) num_peers, group_peer_IDs, &temp_group);
		group_worker->set_group_peers_comm(world, temp_group);

		MPI_Group_free(&world_group);
		MPI_Group_free(&temp_group);

		group_worker->start();
	}
}

void Worker::print_info()
{
	log->info("Worker {}: {} {}:{}", ID, hostname, address, port);
}


//
////int Worker::end_sessions()
////{
////
////	return 0;
////}

//
//
//string Worker::list_sessions()
//{
//	std::stringstream list_of_workers;
//	list_of_workers << "List of session:" << std::endl;
//
//	return list_of_workers.str();
//
//}
//
//int Worker::run_task() {
//
//
//	return 0;
//}
//
//// ----------------------------------------   Parameters   ---------------------------------------
//
//int Worker::process_input_parameters(Parameters & input_parameters) {
//
//
//
//	return 0;
//}
//
//int Worker::process_output_parameters(Parameters & output_parameters) {
//
//
//
//	return 0;
//}
//
//// -----------------------------------------   Library   -----------------------------------------
//
//
//int Worker::print_num_sessions()
//{
//	if (sessions.size() == 0)
//		log->info("No active sessions");
//	else if (sessions.size() == 1)
//		log->info("1 active session");
//	else
//		log->info("{} active sessions", sessions.size());
//
//	return 0;
//}
//
//int Worker::add_session()
//{
//	print_num_sessions();
//
//	return 0;
//}
//
//int Worker::remove_session()
//{
//	print_num_sessions();
//
//	return 0;
//}
//
//int Worker::load_library() {
//
//
//
//	return 0;
//}
//
//// -----------------------------------------   Testing   -----------------------------------------
//
//
//// ----------------------------------------   Matrices   -----------------------------------------
//
////MatrixHandle Worker::register_matrix(size_t num_rows, size_t num_cols) {
////
////	MatrixHandle handle{0};
////
////
////	return handle;
////}
//
//
//int Worker::new_matrix()
//{
//	log->info("Creating new Elemental distributed matrix");
//
//	uint64_t num_rows, num_cols;
//	Matrix_ID ID;
//
//	MPI_Bcast(&ID, 1, MPI_UNSIGNED_SHORT, 0, group);
//	MPI_Bcast(&num_rows, 1, MPI_UNSIGNED_LONG, 0, group);
//	MPI_Bcast(&num_cols, 1, MPI_UNSIGNED_LONG, 0, group);
//
//	MPI_Barrier(group);
//
//	matrices.insert(std::make_pair(ID, DistMatrix_ptr(new El::DistMatrix<double, El::VR, El::STAR>(num_rows, num_cols, grid))));
//	El::Zero(*matrices[ID]);
//	log->info("{} Created new Elemental distributed matrix {}", "pol", ID);
//
//	MPI_Barrier(group);
//
//	return 0;
//}
//
//
//int Worker::get_matrix_layout()
//{
////	std::clock_t start = std::clock();
//
//	log->info("Creating vector of local rows");
//
//	Matrix_ID ID;
//
//	MPI_Bcast(&ID, 1, MPI_UNSIGNED_SHORT, 0, group);
//	MPI_Barrier(group);
//
//	uint64_t num_local_rows = (uint64_t) matrices[ID]->LocalHeight();
//	uint64_t * local_rows = new uint64_t[num_local_rows];
//
//	std::stringstream ss;
//	ss << "Local rows (" << matrices[ID]->LocalHeight() << "): ";
//	for (uint64_t i = 0; i < num_local_rows; i++) {
//		local_rows[i] = (uint64_t) matrices[ID]->GlobalRow(i);
//		ss << "(" << matrices[ID]->GlobalRow(i) << ", " << local_rows[i] << ") ";
//	}
//	log->info(ss.str());
//
//	MPI_Send(&num_local_rows, 1, MPI_UNSIGNED_LONG, 0, 0, group);
////	MPI_Send(local_rows, (int) num_local_rows, MPI_UNSIGNED_LONG, 0, 0, world);		// For some reason this doesn't work
//	for (uint64_t i = 0; i < num_local_rows; i++)
//		MPI_Send(&local_rows[i], 1, MPI_UNSIGNED_LONG, 0, 0, group);
//
////	log->info("DURATION: {}", ( std::clock() - start ) / (double) CLOCKS_PER_SEC);
//
//	delete [] local_rows;
//
//	MPI_Barrier(group);
//
//	return 0;
//}
//
//void Worker::set_value(Matrix_ID ID, uint64_t row, uint64_t col, float value)
//{
//	matrices[ID]->Set(row, col, value);
//}
//
//void Worker::set_value(Matrix_ID ID, uint64_t row, uint64_t col, double value)
//{
//	matrices[ID]->Set(row, col, value);
//}
//
//void Worker::get_value(Matrix_ID ID, uint64_t row, uint64_t col, float & value)
//{
//	value = matrices[ID]->Get(row, col);
//}
//
//void Worker::get_value(Matrix_ID ID, uint64_t row, uint64_t col, double & value)
//{
//	value = matrices[ID]->Get(row, col);
//}
//
//
//int Worker::receive_new_matrix() {
//
//
//	return 0;
//}
//
//int Worker::get_matrix_dimensions() {
//
//
//	return 0;
//}
//
//int Worker::get_transpose() {
//
//
//
//	return 0;
//}
//
//int Worker::matrix_multiply()
//{
//
//
//
//	return 0;
//}
//
//int Worker::get_matrix_rows()
//{
//
//
//
//	return 0;
//}
//
//void Worker::print_data(Matrix_ID ID)
//{
////	std::stringstream ss;
////	ss << "LOCAL DATA:" << std::endl;
////	ss << "Local size: " << matrices[ID]->LocalHeight() << " x " << matrices[ID]->LocalWidth() << std::endl;
////	for (El::Int i = 0; i < matrices[ID]->LocalHeight(); i++) {
////		for (El::Int j = 0; j < matrices[ID]->LocalWidth(); j++)
////			ss <<  matrices[ID]->GetLocal(i, j) << " ";
////		ss << std::endl;
////	}
////	log->info(ss.str());
//}


// ===============================================================================================
// ===============================================================================================

} // namespace alchemist
