#include "GroupWorker.hpp"

namespace alchemist {

//GroupWorker::GroupWorker(Group_ID _group_ID, Worker_ID _worker_ID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//		const unsigned int port, Log_ptr & _log) :
//    	    GroupWorker(_group_ID, _worker_ID, _group, _group_peers, _io_context, tcp::endpoint(tcp::v4(), port), _log) { }
//
//GroupWorker::GroupWorker(Group_ID _group_ID, Worker_ID _worker_ID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//		const tcp::endpoint & endpoint, Log_ptr & _log) :
//			Server(_io_context, endpoint, _log), group_ID(_group_ID), worker_ID(_worker_ID), group(_group), group_peers(_group_peers),
//			next_session_ID(0), accept_connections(false)

GroupWorker::GroupWorker(Group_ID _group_ID, Worker_ID _worker_ID, io_context & _io_context, const unsigned int port, Log_ptr & _log) :
    	    GroupWorker(_group_ID, _worker_ID, _io_context, tcp::endpoint(tcp::v4(), port), _log) { }

GroupWorker::GroupWorker(Group_ID _group_ID, Worker_ID _worker_ID, io_context & _io_context, const tcp::endpoint & endpoint, Log_ptr & _log) :
			Server(_io_context, endpoint, _log), library(nullptr), group_ID(_group_ID), group(MPI_COMM_NULL), group_peers(MPI_COMM_NULL), worker_ID(_worker_ID),
			next_session_ID(0), accept_connections(false)
{
	char buffer[13];
	sprintf(buffer, "worker-%03d", worker_ID);

	log = start_log(string(buffer), "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
	Server::set_log(log);


	log->info("D1111 {}", worker_ID);
}

GroupWorker::~GroupWorker() { }

void GroupWorker::set_group_comm(MPI_Comm & world, MPI_Group & temp_group)
{

	int rank, size;

	log->info("qqqq 1");
	MPI_Comm_create_group(world, temp_group, 0, &group);

	MPI_Comm_rank(group, &rank);
	MPI_Comm_size(group, &size);

	log->info("qqqq 2 {}/{}", rank, size);

}

void GroupWorker::set_group_peers_comm(MPI_Comm & world, MPI_Group & temp_group)
{
	int rank, size;

	MPI_Comm_create_group(world, temp_group, 0, &group_peers);

	grid = new El::Grid(El::mpi::Comm(group_peers));

	MPI_Comm_rank(group_peers, &rank);
	MPI_Comm_size(group_peers, &size);

	log->info("yyyyy {}/{}", rank, size);
}

void GroupWorker::say_something()
{
	log->info("SOMETHING");

}

Worker_ID GroupWorker::get_worker_ID()
{
	return worker_ID;
}

Group_ID GroupWorker::get_group_ID()
{
	return worker_ID;
}

int GroupWorker::start()
{
	wait_for_command();
	return 0;
}


int GroupWorker::close_client()
{

	MPI_Comm_free(&group);
	MPI_Comm_free(&group_peers);

	return 0;
}

// ===============================================================================================
// ====================================   UTILITY FUNCTIONS   ====================================

// ----------------------------------------   File I/O   ----------------------------------------

int GroupWorker::read_HDF5() {

	log->info("GroupWorker::read_HDF5 not yet implemented");

	return 0;
}

// ===============================================================================================
// ====================================   COMMAND FUNCTIONS   ====================================

int GroupWorker::wait_for_command()
{
	alchemist_command c;

	bool should_exit = false;
	int flag = 0;
	MPI_Request req = MPI_REQUEST_NULL;
	MPI_Status status;

	while (!should_exit) {

		MPI_Ibcast(&c, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
		while (flag == 0) {
			MPI_Test(&req, &flag, &status);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		threads.push_back(std::thread(&GroupWorker::handle_command, this, c));

		flag = 0;
		c = IDLE;

		log->info("Number of threads: {}", threads.size());
	}

	for (auto & t: threads) t.join();

	return 0;
}

int GroupWorker::handle_command(alchemist_command c)
{

	switch (c) {
		case IDLE:
			break;
//		case NEW_SESSION:
//			new_session();
//			break;
//		case END_SESSION:
//			end_session();
//			break;
		case SAY_SOMETHING:
			say_something();
			break;
		case ACCEPT_CONNECTION:
			Group_ID ID;
			MPI_Bcast(&ID, 1, MPI_UNSIGNED_SHORT, 0, group);
			log->info("{} {}", group_ID, ID);
			accept_connection();
			break;
		case NEW_MATRIX:
			new_matrix();
			break;
		case CLIENT_MATRIX_LAYOUT:
			get_matrix_layout();
			break;
		case WORKER_LOAD_LIBRARY:
			load_library();
			break;
		case WORKER_RUN_TASK:
			run_task();
			break;
	}

	return 0;
}

//int GroupWorker::new_client()
//{
//	log->info("Preparing for new client");
//	MPI_Status status;
//
//	MPI_Recv(&client_ID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//
//	uint16_t num_peers = 0;
//
//	MPI_Recv(&num_peers, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//
//	uint16_t peer_ID;
//	const int group_IDs[num_peers+1];
//	const int group_peer_IDs[num_peers];
//
//	group_IDs[0] = 0;			// First member is the driver
//	for (uint16_t i = 0; i < num_peers; i++) {
//		MPI_Recv(&peer_ID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//		group_IDs[i+1] = peer_ID;
//		group_peer_IDs[i] = peer_ID;
//	}
//
//	MPI_Group world_group;
//	MPI_Group temp_group;
//
//	MPI_Comm_group(world, &world_group);
//
//	MPI_Group_incl(world_group, (int) (num_peers+1), group_IDs, &temp_group);
//	MPI_Comm_create_group(world, temp_group, 0, &group);			// Create a new communicator based on the group_IDs
//
//	MPI_Group_incl(world_group, (int) num_peers, group_peer_IDs, &temp_group);
//	MPI_Comm_create_group(world, temp_group, 0, &group_peers);		// Create a new communicator based on the group_peer_IDs
//
//	grid = El::mpi::Comm(group_peers);
//
//	return 0;
//}

//int GroupWorker::end_sessions()
//{
//
//	return 0;
//}


int GroupWorker::receive_test_string(const char * data, const uint32_t length)
{
	log->info("{} Test string: {}", client_preamble(), string(data, length));

	return 0;
}

string GroupWorker::get_test_string()
{
	std::stringstream ss;
	ss << "This is a test string from Alchemist worker " << worker_ID;

	return ss.str();
}


//string GroupWorker::preamble()
//{
//	return client_preamble() + " " + session_preamble();
//}

string GroupWorker::client_preamble()
{
	std::stringstream ss;

	ss << "[Job " << group_ID << " (" << address.c_str() << ":" << port << ")]";

	return ss.str();
}
//
//string GroupWorker::session_preamble()
//{
//	std::stringstream ss;
//
//	ss << "[Session " << session_ID << "]";
//
//	return ss.str();
//}

int GroupWorker::load_library()
{
	uint16_t library_name_length = 0;
	uint16_t library_path_length = 0;

	MPI_Bcast(&library_name_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	char library_name_c[library_name_length+1];
	MPI_Bcast(library_name_c, library_name_length+1, MPI_CHAR, 0, group);

	MPI_Bcast(&library_path_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	char library_path_c[library_path_length+1];
	MPI_Bcast(library_path_c, library_path_length+1, MPI_CHAR, 0, group);

	string library_name = string(library_name_c);
	string library_path = string(library_path_c);

	char * cstr = new char [library_path.length()+1];
	std::strcpy(cstr, library_path.c_str());

	log->info("Loading library {} located at {}", library_name, library_path);

	void * lib = dlopen(library_path.c_str(), RTLD_NOW);
	if (lib == NULL) {
		log->info("dlopen failed: {}", dlerror());

		return -1;
	}

	dlerror();			// Reset errors

	create_t * create_library = (create_t*) dlsym(lib, "create");
	const char * dlsym_error = dlerror();
	if (dlsym_error) {
//    		log->info("dlsym with command \"load\" failed: {}", dlerror());

//        	delete [] cstr;
//        	delete dlsym_error;

		return -1;
	}

	library = create_library(group);

//    	libraries.insert(std::make_pair(library_name, LibraryInfo(library_name, library_path, lib, library)));

//	if (!library->load()) {
//		log->info("Library {} loaded", library_name);
//
//		Parameters input;
//		Parameters output;
//
//		library->run("greet", input, output);
//	}

	delete [] cstr;
	delete dlsym_error;

	MPI_Barrier(group);

	return 0;
}

int GroupWorker::new_matrix()
{
	log->info("Creating new Elemental distributed matrix");

	uint64_t num_rows, num_cols;
	Matrix_ID ID;

	MPI_Bcast(&ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&num_rows, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&num_cols, 1, MPI_UNSIGNED_LONG, 0, group);

	MPI_Barrier(group);

//	matrices.insert(std::make_pair(ID, new DistMatrix(num_rows, num_cols, group_peers)));
	matrices.insert(std::make_pair(ID, new DistMatrix(num_rows, num_cols, *grid)));
	log->info("{} Created new Elemental distributed matrix {}", client_preamble(), ID);

	MPI_Barrier(group);

	return 0;
}

int GroupWorker::get_matrix_layout()
{
//	std::clock_t start = std::clock();

	log->info("Creating vector of local rows");

	Matrix_ID ID;

	MPI_Bcast(&ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Barrier(group);

	auto matrix = matrices[ID]->data;
	uint64_t num_local_rows = (uint64_t) matrix->LocalHeight();
	uint64_t * local_rows = new uint64_t[num_local_rows];

//	std::stringstream ss;
//	ss << "Local rows (" <<matrix->LocalHeight() << "): ";
	for (uint64_t i = 0; i < num_local_rows; i++) {
		local_rows[i] = (uint64_t) matrix->GlobalRow(i);
//		ss << "(" << i << ", " << local_rows[i] << ") ";
	}
//	log->info(ss.str());

	MPI_Send(&num_local_rows, 1, MPI_UNSIGNED_LONG, 0, 0, group);
//	MPI_Send(local_rows, (int) num_local_rows, MPI_UNSIGNED_LONG, 0, 0, world);		// For some reason this doesn't work
	for (uint64_t i = 0; i < num_local_rows; i++)
		MPI_Send(&local_rows[i], 1, MPI_UNSIGNED_LONG, 0, 0, group);

//	log->info("DURATION: {}", ( std::clock() - start ) / (double) CLOCKS_PER_SEC);

	delete [] local_rows;

	MPI_Barrier(group);

	return 0;
}

void GroupWorker::set_value(Matrix_ID ID, uint64_t row, uint64_t col, float value)
{
	matrices[ID]->data->Set(row, col, value);
}

void GroupWorker::set_value(Matrix_ID ID, uint64_t row, uint64_t col, double value)
{
	matrices[ID]->data->Set(row, col, value);
}

void GroupWorker::get_value(Matrix_ID ID, uint64_t row, uint64_t col, float & value)
{
	value = matrices[ID]->data->Get(row, col);
}

void GroupWorker::get_value(Matrix_ID ID, uint64_t row, uint64_t col, double & value)
{
	value = matrices[ID]->data->Get(row, col);
}

void GroupWorker::print_data(Matrix_ID ID)
{
	std::stringstream ss;
	ss << "LOCAL DATA:" << std::endl;
	ss << "Local size: " << matrices[ID]->data->LocalHeight() << " x " << matrices[ID]->data->LocalWidth() << std::endl;
	for (El::Int i = 0; i < matrices[ID]->data->LocalHeight(); i++) {
		for (El::Int j = 0; j < matrices[ID]->data->LocalWidth(); j++)
			ss <<  matrices[ID]->data->GetLocal(i, j) << " ";
		ss << std::endl;
	}
	log->info(ss.str());
}


string GroupWorker::list_sessions()
{
	std::stringstream list_of_workers;
	list_of_workers << "List of session:" << std::endl;

	return list_of_workers.str();

}

int GroupWorker::run_task()
{
	Matrix_ID matrix_ID;
	uint32_t rank;
	uint8_t method;

	MPI_Bcast(&matrix_ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&rank, 1, MPI_UNSIGNED, 0, group);
	MPI_Bcast(&method, 1, MPI_UNSIGNED_CHAR, 0, group);

	MPI_Barrier(group);

//	MPI_Bcast(&data_length, 1, MPI_UNSIGNED, 0, group);
//	char data[data_length];
//	MPI_Bcast(&data[0], data_length, MPI_CHAR, 0, group);
//
//	MPI_Barrier(group);
//
//	log->info("9999999");
//	Message temp_msg = Message();
//	log->info("8888888");
//	temp_msg.copy_data(&data[0], data_length);
//	log->info("6666666");
//	temp_msg.decode_header();
//	temp_msg.cl = sessions[temp_msg.session_ID+1]->read_msg.cl;
//	temp_msg.to_string();

//	if (!library) {
//		string task = temp_msg.read_string();
//		Matrix_ID matrix_ID = temp_msg.read_uint16();
//		uint32_t rank = temp_msg.read_uint16();

		truncated_SVD(matrix_ID, rank, method);
//	}

	return 0;
}

int GroupWorker::truncated_SVD(Matrix_ID matrix_ID, uint32_t rank, uint8_t method)
{
	auto workingMat = matrices[matrix_ID]->data;

	int m = workingMat->Height();
	int n = workingMat->Width();

	MPI_Barrier(group);

	log->info("Starting truncated SVD");

//	  int LOCALEIGS = 0; // TODO: make these an enumeration, and global to Alchemist
//	  int LOCALEIGSPRECOMPUTE = 1;
//	  int DISTEIGS = 2;

	// Assume matrix is row-partitioned b/c relaying it out doubles memory requirements

	//NB: sometimes it makes sense to precompute the gramMat (when it's cheap (we have a lot of cores and enough memory), sometimes
	// it makes more sense to compute A'*(A*x) separately each time (when we don't have enough memory for gramMat, or its too expensive
	// time-wise to precompute GramMat). trade-off depends on k (through the number of Arnoldi iterations we'll end up needing), the
	// amount of memory we have free to store GramMat, and the number of cores we have available
	El::Matrix<double> localGramChunk;

	if (method == 1) {
		localGramChunk.Resize(n, n);
		log->info("Computing the local contribution to A'*A");
		log->info("Local matrix's dimensions are {}, {}", workingMat->LockedMatrix().Height(), workingMat->LockedMatrix().Width());
		log->info("Storing A'*A in {},{} matrix", n, n);
		auto startFillLocalMat = std::chrono::system_clock::now();
		if (workingMat->LockedMatrix().Height() > 0)
			El::Gemm(El::TRANSPOSE, El::NORMAL, 1.0, workingMat->LockedMatrix(), workingMat->LockedMatrix(), 0.0, localGramChunk);
		else
			El::Zeros(localGramChunk, n, n);
		std::chrono::duration<double, std::milli> fillLocalMat_duration(std::chrono::system_clock::now() - startFillLocalMat);
		log->info("Took {} ms to compute local contribution to A'*A", fillLocalMat_duration.count());
	}

	uint8_t command;
	std::unique_ptr<double[]> vecIn{new double[n]};
	El::Matrix<double> localx(n, 1);
	El::Matrix<double> localintermed(workingMat->LocalHeight(), 1);
	El::Matrix<double> localy(n, 1);
	localx.LockedAttach(n, 1, vecIn.get(), 1);
	auto distx = new DistMatrix(n, 1, *grid);
	auto distintermed = new DistMatrix(m, 1, *grid);
//	auto distx = El::DistMatrix<double, El::STAR, El::STAR>(n, 1, self->grid);
//	auto distintermed = El::DistMatrix<double, El::STAR, El::STAR>(m, 1, self->grid);

	log->info("finished initialization for truncated SVD");

	while(true) {
		MPI_Bcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group);
//		mpi::broadcast(self->world, command, 0);
		if (command == 1 && method == 0) {
			void * uut;
			MPI_Bcast(vecIn.get(), n, MPI_DOUBLE, 0, group);
			auto uu = vecIn.get();
//			log->info("Cvvvv {} {}", uu[0], uu[1]);
//			mpi::broadcast(self->world, vecIn.get(), n, 0);
			El::Gemv(El::NORMAL, 1.0, workingMat->LockedMatrix(), localx, 0.0, localintermed);
//			log->info("Cvvrr {} {}", localx(0,0), localx(1,0));
			El::Gemv(El::TRANSPOSE, 1.0, workingMat->LockedMatrix(), localintermed, 0.0, localy);
			MPI_Reduce(localy.LockedBuffer(), uut, n, MPI_DOUBLE, MPI_SUM, 0, group);
//			mpi::reduce(self->world, localy.LockedBuffer(), n, std::plus<double>(), 0);
		}
		if (command == 1 && method == 1) {
			MPI_Bcast(vecIn.get(), n, MPI_DOUBLE, 0, group);
			void * uut;
//			mpi::broadcast(self->world, vecIn.get(), n, 0);
			El::Gemv(El::NORMAL, 1.0, localGramChunk, localx, 0.0, localy);
			MPI_Reduce(localy.LockedBuffer(), uut, n, MPI_DOUBLE, MPI_SUM, 0, group);
//			mpi::reduce(self->world, localy.LockedBuffer(), n, std::plus<double>(), 0);
		}
		if (command == 1 && method == 2) {
//			El::Zeros(distx, n, 1);
//			log->info("Computing a mat-vec prod against A^TA");
//			if (self->world.rank() == 1) {
//				self->world.recv(0, 0, vecIn.get(), n);
//				distx.Reserve(n);
//				for(El::Int row=0; row < n; row++)
//					distx.QueueUpdate(row, 0, vecIn[row]);
//			}
//			else {
//				distx.Reserve(0);
//			}
//			distx.ProcessQueues();
//			log->info("Retrieved x, computing A^TAx");
//			El::Gemv(El::NORMAL, 1.0, *workingMat, distx, 0.0, distintermed);
//			log->info("Computed y = A*x");
//			El::Gemv(El::TRANSPOSE, 1.0, *workingMat, distintermed, 0.0, distx);
//			log->info("Computed x = A^T*y");
//			if(self->world.rank() == 1) {
//				world.send(0, 0, distx.LockedBuffer(), n);
//			}
		}
		if (command == 2) {
			uint32_t nconv;
			MPI_Bcast(&nconv, 1, MPI_UNSIGNED, 0, group);

			MatrixXd rightEigs(n, nconv);
			MPI_Bcast(rightEigs.data(), n*nconv, MPI_DOUBLE, 0, group);
//			mpi::broadcast(self->world, rightEigs.data(), n*nconv, 0);
			VectorXd singValsSq(nconv);
			MPI_Bcast(singValsSq.data(), nconv, MPI_DOUBLE, 0, group);
//			mpi::broadcast(self->world, singValsSq.data(), nconv, 0);
			log->info("Received the right eigenvectors and the eigenvalues");

//			auto U = new DistMatrix(m, nconv, group_peers);
//			matrices.insert(std::make_pair(matrix_ID+1, U));
//			auto S = new DistMatrix(nconv, 1, group_peers);
//			matrices.insert(std::make_pair(matrix_ID+2, S));
//			auto Sinv = new DistMatrix(nconv, 1, group_peers);
//			matrices.insert(std::make_pair(matrix_ID+3, Sinv));
//			auto V = new DistMatrix(n, nconv, group_peers);
//			matrices.insert(std::make_pair(matrix_ID+4, V));
			auto U = new DistMatrix(m, nconv, *grid);
			matrices.insert(std::make_pair(matrix_ID+1, U));
			auto S = new DistMatrix(nconv, 1, *grid);
			matrices.insert(std::make_pair(matrix_ID+2, S));
			auto Sinv = new DistMatrix(nconv, 1, *grid);
//			matrices.insert(std::make_pair(matrix_ID+3, Sinv));
			auto V = new DistMatrix(n, nconv, *grid);
			matrices.insert(std::make_pair(matrix_ID+3, V));
//
//			auto U = new El::DistMatrix<double, El::VR, El::STAR>(m, nconv, self->grid);
//			DistMatrix * S = new El::DistMatrix<double, El::VR, El::STAR>(nconv, 1, self->grid);
//			DistMatrix * Sinv = new El::DistMatrix<double, El::VR, El::STAR>(nconv, 1, self->grid);
//			DistMatrix * V = new El::DistMatrix<double, El::VR, El::STAR>(n, nconv, self->grid);
//
//			ENSURE(self->matrices.insert(std::make_pair(UHandle, std::unique_ptr<DistMatrix>(U))).second);
//			ENSURE(self->matrices.insert(std::make_pair(SHandle, std::unique_ptr<DistMatrix>(S))).second);
//			ENSURE(self->matrices.insert(std::make_pair(VHandle, std::unique_ptr<DistMatrix>(V))).second);
			log->info("Created new matrix objects to hold U,S,V");
//
			// populate V
			for(El::Int rowIdx=0; rowIdx < n; rowIdx++)
				for(El::Int colIdx=0; colIdx < (El::Int) nconv; colIdx++)
					if(V->data->IsLocal(rowIdx, colIdx))
						V->data->SetLocal(V->data->LocalRow(rowIdx), V->data->LocalCol(colIdx), rightEigs(rowIdx,colIdx));
			rightEigs.resize(0,0); // clear any memory this temporary variable used (a lot, since it's on every rank)

			// populate S, Sinv
			for(El::Int idx=0; idx < (El::Int) nconv; idx++) {
				if(S->data->IsLocal(idx, 0)) {
//					log->info("BBBBBBBBBBB {} {} {}", idx, S->data->LocalRow(idx), singValsSq(idx));
					S->data->SetLocal(S->data->LocalRow(idx), 0, std::sqrt(singValsSq(idx)));
				}
				if(Sinv->data->IsLocal(idx, 0))
					Sinv->data->SetLocal(Sinv->data->LocalRow(idx), 0, 1/std::sqrt(singValsSq(idx)));
			}
			log->info("Stored V and S");

			// form U
			log->info("computing A*V = U*Sigma");
			log->info("A is {}-by-{}, V is {}-by-{}, the resulting matrix should be {}-by-{}", workingMat->Height(), workingMat->Width(), V->data->Height(), V->data->Width(), U->data->Height(), U->data->Width());
			//Gemm(1.0, *workingMat, *V, 0.0, *U, self->log);
			El::Gemm(El::NORMAL, El::NORMAL, 1.0, *workingMat, *(V->data), 0.0, *(U->data));
			log->info("done computing A*V, rescaling to get U");
			// TODO: do a QR instead to ensure stability, but does column pivoting so would require postprocessing S,V to stay consistent
			El::DiagonalScale(El::RIGHT, El::NORMAL, *(Sinv->data), *(U->data));
			log->info("Computed and stored U");

			break;
		}
	}


	MPI_Barrier(group);

	return 0;
}

// ----------------------------------------   Parameters   ---------------------------------------

int GroupWorker::process_input_parameters(Parameters & input_parameters) {



	return 0;
}

int GroupWorker::process_output_parameters(Parameters & output_parameters) {



	return 0;
}

// -----------------------------------------   Library   -----------------------------------------


int GroupWorker::print_num_sessions()
{
	if (sessions.size() == 0)
		log->info("No active session");
	else if (sessions.size() == 1)
		log->info("1 active session");
	else
		log->info("{} active sessions", sessions.size() );

	return 0;
}

int GroupWorker::add_session()
{
	print_num_sessions();

	return 0;
}

int GroupWorker::remove_session()
{
//	session = nullptr;

	print_num_sessions();

	return 0;
}

//int GroupWorker::load_library() {
//
//
//
//	return 0;
//}

// -----------------------------------------   Testing   -----------------------------------------


// ----------------------------------------   Matrices   -----------------------------------------

//MatrixHandle GroupWorker::register_matrix(size_t num_rows, size_t num_cols) {
//
//	MatrixHandle handle{0};
//
//
//	return handle;
//}

int GroupWorker::receive_new_matrix() {


	return 0;
}

int GroupWorker::get_matrix_dimensions() {


	return 0;
}

int GroupWorker::get_transpose() {



	return 0;
}

int GroupWorker::matrix_multiply()
{



	return 0;
}

int GroupWorker::get_matrix_rows()
{



	return 0;
}


int GroupWorker::new_session(tcp::socket socket)
{
	next_session_ID++;
	auto session_ptr = std::make_shared<WorkerSession>(std::move(socket), *this, next_session_ID, group_ID, log);
	sessions.insert(std::make_pair(next_session_ID, session_ptr));
	sessions[next_session_ID]->start();

	return 0;
}

int GroupWorker::accept_connection()
{
	acceptor.async_accept(
		[this](error_code ec, tcp::socket socket)
		{
//			if (!ec) std::make_shared<WorkerSession>(std::move(socket), *this, next_session_ID++, log)->start();
			if (!ec) new_session(std::move(socket));

			accept_connection();
		});

	ic.run();

	return 0;
}

//int GroupWorker::start_new_session()
//{
//	next_session_ID++;
//	session = std::make_shared<WorkerSession>(std::move(socket), *this, next_session_ID, log);
//
//	return 0;
//}
//
//int GroupWorker::accept_connection()
//{
//	acceptor.async_accept(
//		[this](error_code ec, tcp::socket socket)
//		{
////			if (!ec) std::make_shared<GroupWorkerSession>(std::move(socket), *this, next_session_ID++, log)->start();
//			if (!ec) start_new_session();
//
//			accept_connection();
//		});
//
//	ic.run();
//
//	return 0;
//}

// ===============================================================================================
// ===============================================================================================

} // namespace alchemist
