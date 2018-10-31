#include "GroupDriver.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

GroupDriver::GroupDriver(Group_ID _ID, Driver & _driver): ID(_ID), driver(_driver), group(MPI_COMM_NULL), cl(SCALA), next_matrix_ID(1) { }

GroupDriver::GroupDriver(Group_ID _ID, Driver & _driver, Log_ptr & _log): ID(_ID), driver(_driver), group(MPI_COMM_NULL),
		log(_log), cl(SCALA), library(nullptr), next_matrix_ID(0) { }

GroupDriver::~GroupDriver() { }

void GroupDriver::start(tcp::socket socket)
{
	log->info("DEBUG: GroupDriver : Start");

	session = std::make_shared<DriverSession>(std::move(socket), *this, ID, log);
	session->start();
}

map<Worker_ID, WorkerInfo> GroupDriver::allocate_workers(const uint16_t & num_requested_workers)
{

	workers = driver.allocate_workers(ID, num_requested_workers);

	alchemist_command command = ACCEPT_CONNECTION;

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	MPI_Bcast(&ID, 1, MPI_UNSIGNED_SHORT, 0, group);

	return workers;
}

void GroupDriver::say_something()
{
	log->info("SOMETHING");

	alchemist_command command = SAY_SOMETHING;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

}

uint64_t GroupDriver::get_num_rows(Matrix_ID & matrix_ID)
{
	return matrices[matrix_ID].num_rows;
}

uint64_t GroupDriver::get_num_cols(Matrix_ID & matrix_ID)
{
	return matrices[matrix_ID].num_cols;
}

void GroupDriver::set_group_comm(MPI_Comm & world, MPI_Group & temp_group)
{
	MPI_Comm_create_group(world, temp_group, 0, &group);
}

string GroupDriver::list_sessions()
{
//	std::stringstream list_of_sessions;
//	list_of_sessions << "List of current sessions:" << std::endl;
//	log->info("List of current sessions:");
//
//	if (workers.size() == 0)
//		list_of_sessions << "    No active sessions" << std::endl;
//
//	return list_of_sessions.str();

	return "NOWE";

//	for(auto it = sessions.begin(); it != sessions.end(); ++it)
//		log->info("        Session {}: {}:{}, assigned to Session {}", it->first, workers[it->first].hostname, workers[it->first].port, it->second);
}



// -----------------------------------------   Workers   -----------------------------------------

int GroupDriver::load_library(string library_path, string library_name)
{
	log->info("Loading library {} located at {}", library_name, library_path);

	alchemist_command command = WORKER_LOAD_LIBRARY;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	uint16_t library_name_length = library_name.length();
	uint16_t library_path_length = library_path.length();

	char library_name_c[library_name_length+1];
	char library_path_c[library_path_length+1];

	    // copying the contents of the
	    // string to char array
	strcpy(library_name_c, library_name.c_str());
	strcpy(library_path_c, library_path.c_str());

//	auto library_name_c = library_name.c_str();
//	auto library_path_c = library_path.c_str();
	uint16_t library_name_c_length = strlen(library_name_c);
	uint16_t library_path_c_length = strlen(library_path_c);

//	log->info("FGG {} {}", library_name_c, library_path_c);

	MPI_Bcast(&library_name_c_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(library_name_c, library_name_length+1, MPI_CHAR, 0, group);
	MPI_Bcast(&library_path_c_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(library_path_c, library_path_length+1, MPI_CHAR, 0, group);

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

//int GroupDriver::run_task(const char * data, uint32_t data_length)
//{
//	alchemist_command command = WORKER_RUN_TASK;
//
//	log->info("Sending command {} to workers", get_command_name(command));
//
//	MPI_Request req;
//	MPI_Status status;
//	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
//	MPI_Wait(&req, &status);
//
//	MPI_Bcast(&data_length, 1, MPI_UNSIGNED, 0, group);
//	MPI_Bcast(&data, data_length, MPI_CHAR, 0, group);
//
//	MPI_Barrier(group);
//
//	Message temp_msg = Message();
//	temp_msg.cl = session->read_msg.cl;
//	temp_msg.copy_data(&data[0], data_length);
//	temp_msg.to_string();
//
//	if (!library) {
//		string task = temp_msg.read_string();
//		Matrix_ID matrix_ID = temp_msg.read_uint16();
//		uint32_t rank = temp_msg.read_uint16();
//
//		truncated_SVD(matrix_ID, rank);
//	}
//
//	return 0;
//}

int GroupDriver::run_task(string task, Matrix_ID matrix_ID, uint32_t rank, uint8_t method)
{
	alchemist_command command = WORKER_RUN_TASK;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	MPI_Bcast(&matrix_ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&rank, 1, MPI_UNSIGNED, 0, group);
	MPI_Bcast(&method, 1, MPI_UNSIGNED_CHAR, 0, group);

	MPI_Barrier(group);
//
//	Message temp_msg = Message();
//	temp_msg.cl = session->read_msg.cl;
//	temp_msg.copy_data(&data[0], data_length);
//	temp_msg.to_string();

//	if (!library) {
//		string task = temp_msg.read_string();
//		Matrix_ID matrix_ID = temp_msg.read_uint16();
//		uint32_t rank = temp_msg.read_uint16();

		truncated_SVD(matrix_ID, rank, method);
//	}

	return 0;
}

int GroupDriver::truncated_SVD(Matrix_ID matrix_ID, uint32_t rank, uint8_t method)
{
	uint64_t m = matrices[matrix_ID].num_rows;
	uint64_t n = matrices[matrix_ID].num_cols;

	log->info("Starting truncated SVD on {}x{} matrix", m, n);
	log->info("Settings:");
	log->info("    rank = {}", rank);

	MPI_Barrier(group);

//	int LOCALEIGS = 0; // TODO: make these an enumeration, and global to Alchemist
//	int LOCALEIGSPRECOMPUTE = 1;
//	int DISTEIGS = 2;

//	MatrixHandle UHandle{nextMatrixId++};
//	MatrixHandle SHandle{nextMatrixId++};
//	MatrixHandle VHandle{nextMatrixId++};
//
//	TruncatedSVDCommand cmd(inputMat, UHandle, SHandle, VHandle, k, method);
//	issue(cmd);

	switch(method) {
	case 2:
		log->info("using distributed mat-vec prods against A, then A tranpose");
		break;
	case 1:
		log->info("using local mat-vec prods computed on the fly against the local Gramians");
		break;
	case 0:
		log->info("using local mat-vec prods against the precomputed local Gramians");
		break;
	}

	ARrcSymStdEig<double> prob(n, rank, "LM");
	uint8_t command;
	std::vector<double> zerosVector(n);
	for(uint32_t idx = 0; idx < n; idx++)
		zerosVector[idx] = 0;

	uint32_t iterNum = 0;

	while (!prob.ArnoldiBasisFound()) {
		prob.TakeStep();
		++iterNum;
		if(iterNum % 20 == 0) log->info("Computed {} mv products", iterNum);
		if (prob.GetIdo() == 1 || prob.GetIdo() == -1) {
			command = 1;

			MPI_Bcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group);
//			mpi::broadcast(world, command, 0);
			if (method == 0 || method == 1) {
				auto temp = prob.GetVector();
//				log->info("DDDDD1 {} {} {}", iterNum, temp[0], temp[1]);
				MPI_Bcast(prob.GetVector(), n, MPI_DOUBLE, 0, group);
//				mpi::broadcast(world, prob.GetVector(), n, 0);
//				auto z = zerosVector.data();
				MPI_Reduce(zerosVector.data(), prob.PutVector(), n, MPI_DOUBLE, MPI_SUM, 0, group);
				auto temp1 = prob.GetVector();
//				log->info("DDDDD2 {} {} {}", iterNum, temp1[0], temp1[1]);
//				mpi::reduce(world, zerosVector.data(), n, prob.PutVector(), std::plus<double>(), 0);
			}
			if (method == 2) {
//				MPI_Status status;
//				MPI_Send(prob.GetVector(), n, MPI_DOUBLE, 1, 0, group);
//				MPI_Recv(prob.PutVector(), n, MPI_DOUBLE, 1, 0, group, status);
////				world.send(1, 0, prob.GetVector(), n);
////				world.recv(1, 0, prob.PutVector(), n);
			}
		}
	}

	prob.FindEigenvectors();
	uint32_t nconv = prob.ConvergedEigenvalues();
	uint32_t niters = prob.GetIter();
	log->info("Done after {} Arnoldi iterations, converged to {} eigenvectors of size {}", niters, nconv, n);

	//NB: it may be the case that n*nconv > 4 GB, then have to be careful!
	// assuming tall and skinny A for now
	MatrixXd rightVecs(n, nconv);
	log->info("Allocated matrix for right eigenvectors of A'*A");
	// Eigen uses column-major layout by default!
	for(uint32_t idx = 0; idx < nconv; idx++)
		std::memcpy(rightVecs.col(idx).data(), prob.RawEigenvector(idx), n*sizeof(double));
	log->info("Copied right eigenvectors into allocated storage");

	// Populate U, V, S
	command = 2;
	MPI_Bcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group);
	MPI_Bcast(&nconv, 1, MPI_UNSIGNED, 0, group);
//	mpi::broadcast(world, nconv, 0);
	log->info("Broadcasted command and number of converged eigenvectors");
	MPI_Bcast(rightVecs.data(), n*nconv, MPI_DOUBLE, 0, group);
//	mpi::broadcast(world, rightVecs.data(), n*nconv, 0);
	log->info("Broadcasted right eigenvectors");
	auto ng = prob.RawEigenvalues();
//	log->info("BBBBBBBBBBB {}", ng[0]);
	MPI_Bcast(prob.RawEigenvalues(), nconv, MPI_DOUBLE, 0, group);
//	mpi::broadcast(world, prob.RawEigenvalues(), nconv, 0);
	log->info("Broadcasted eigenvalues");


	matrices.insert(std::make_pair(matrix_ID+1, MatrixInfo(matrix_ID+1, m, nconv)));
	matrices.insert(std::make_pair(matrix_ID+2, MatrixInfo(matrix_ID+2, nconv, 1)));
	matrices.insert(std::make_pair(matrix_ID+3, MatrixInfo(matrix_ID+3, n, nconv)));

	next_matrix_ID += 3;

//	MatrixDescriptor Uinfo(UHandle, m, nconv);
//	MatrixDescriptor Sinfo(SHandle, nconv, 1);
//	MatrixDescriptor Vinfo(VHandle, n, nconv);
//	ENSURE(matrices.insert(std::make_pair(UHandle, Uinfo)).second);
//	ENSURE(matrices.insert(std::make_pair(SHandle, Sinfo)).second);
//	ENSURE(matrices.insert(std::make_pair(VHandle, Vinfo)).second);
//
	log->info("Waiting on workers to store U,S,V");
//
//	world.barrier();
//	log->info("Writing ok status followed by U,S,V handles");
//	output.writeInt(0x1);
//	output.writeInt(UHandle.id);
//	output.writeInt(SHandle.id);
//	output.writeInt(VHandle.id);
//	output.flush();

	MPI_Barrier(group);

	return 0;
}

Matrix_ID GroupDriver::new_matrix(unsigned char type, unsigned char layout, uint64_t num_rows, uint64_t num_cols)
{
	alchemist_command command = NEW_MATRIX;

	log->info("Sending command {} to workers", get_command_name(command));

	next_matrix_ID++;

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	int size = 0;
	MPI_Type_size(MPI_UNSIGNED_LONG, &size);

	MPI_Bcast(&next_matrix_ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&num_rows, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&num_cols, 1, MPI_UNSIGNED_LONG, 0, group);

	MPI_Barrier(group);

	matrices.insert(std::make_pair(next_matrix_ID, MatrixInfo(next_matrix_ID, num_rows, num_cols)));

	MPI_Barrier(group);

	return next_matrix_ID;
}

vector<uint16_t> & GroupDriver::get_row_assignments(Matrix_ID & matrix_ID)
{
	return matrices[matrix_ID].row_assignments;
}

void GroupDriver::determine_row_assignments(Matrix_ID & matrix_ID)
{
	MatrixInfo & matrix = matrices[matrix_ID];
	uint64_t worker_num_rows;
	uint64_t * row_indices;

	std::clock_t start;

	alchemist_command command = CLIENT_MATRIX_LAYOUT;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	MPI_Bcast(&matrix.ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Barrier(group);

	for (auto it = workers.begin(); it != workers.end(); it++) {
		Worker_ID id = it->first;
		start = std::clock();
		MPI_Recv(&worker_num_rows, 1, MPI_UNSIGNED_LONG, id, 0, group, &status);
//		log->info("DURATION 1: {}", ( std::clock() - start ) / (double) CLOCKS_PER_SEC);

		start = std::clock();
		row_indices = new uint64_t[worker_num_rows];
//		MPI_Recv(row_indices, worker_num_rows, MPI_UNSIGNED_LONG, id, 0, world, &status);		// For some reason this doesn't work

//		log->info("DURATION 2: {}", ( std::clock() - start ) / (double) CLOCKS_PER_SEC);

		start = std::clock();
		for (uint64_t i = 0; i < worker_num_rows; i++) {
			MPI_Recv(&row_indices[i], 1, MPI_UNSIGNED_LONG, id, 0, group, &status);
			matrix.row_assignments[row_indices[i]] = id;
		}
//		log->info("DURATION 3: {}", ( std::clock() - start ) / (double) CLOCKS_PER_SEC);

		delete [] row_indices;
	}

	MPI_Barrier(group);

//	std::stringstream ss;
//
//	ss << std::endl << "Row | Worker " << std::endl;
//	for (uint64_t row = 0; row < matrix.num_rows; row++) {
//		ss << row << " | " << matrix.row_assignments[row] << std::endl;
//	}
//
//	log->info(ss.str());
}

vector<vector<vector<float> > > GroupDriver::prepare_data_layout_table(uint16_t num_alchemist_workers, uint16_t num_client_workers)
{
	auto data_ratio = float(num_alchemist_workers)/float(num_client_workers);

	vector<vector<vector<float> > > layout_rr = vector<vector<vector<float> > >(num_client_workers, vector<vector<float> >(num_alchemist_workers, vector<float>(2)));

	for (int i = 0; i < num_client_workers; i++)
		for (int j = 0; j < num_alchemist_workers; j++) {
			layout_rr[i][j][0] = 0.0;
			layout_rr[i][j][1] = 0.0;
		}

	int j = 0;
	float diff, col_sum;

	for (int i = 0; i < num_client_workers; i++) {
		auto dr = data_ratio;
		for (; j < num_alchemist_workers; j++) {
			col_sum = 0.0;
			for (int k = 0; k < i; k++)
				col_sum += layout_rr[k][j][1];

			if (i > 0) diff = 1.0 - col_sum;
			else diff = 1.0;

			if (dr >= diff) {
				layout_rr[i][j][1] = layout_rr[i][j][0] + diff;
				dr -= diff;
			}
			else {
				layout_rr[i][j][1] = layout_rr[i][j][0] + dr;
				break;
			}
		}
	}

	for (int i = 0; i < num_client_workers; i++)
		for (int j = 0; j < num_alchemist_workers; j++) {
			layout_rr[i][j][0] /= data_ratio;
			layout_rr[i][j][1] /= data_ratio;
			if (j > 0) {
				layout_rr[i][j][0] += layout_rr[i][j-1][1];
				layout_rr[i][j][1] += layout_rr[i][j-1][1];
				if (layout_rr[i][j][1] >= 0.99) break;
			}
		}

//	for (int i = 0; i < num_client_workers; i++) {
//			for (int j = 0; j < num_alchemist_workers; j++)
//				std::cout << layout_rr[i][j][0] << "," << layout_rr[i][j][1] << " ";
//		std::cout << std::endl;
//	}

	return layout_rr;
}


// ----------------------------------------   File I/O   ----------------------------------------

int GroupDriver::read_HDF5() {

	log->info("GroupDriver::read_HDF5 not yet implemented");



	return 0;
}

// ---------------------------------------   Information   ---------------------------------------


// ----------------------------------------   Parameters   ---------------------------------------

int GroupDriver::process_input_parameters(Parameters & input_parameters) {


	return 0;
}

int GroupDriver::process_output_parameters(Parameters & output_parameters) {



	return 0;
}

// -----------------------------------------   Library   -----------------------------------------


// ----------------------------------------   Matrices   -----------------------------------------


//MatrixHandle Driver::register_matrix(size_t num_rows, size_t num_cols) {
//
//	MatrixHandle handle{next_matrix_ID++};
//
//	return handle;
//}


//int GroupDriver::receive_new_matrix() {
//
//
//	return 0;
//}
//
//
//int GroupDriver::get_matrix_dimensions() {
//
//
//	return 0;
//}
//
//
//int GroupDriver::get_transpose() {
//
//
//	return 0;
//}
//
//
//int GroupDriver::matrix_multiply() {
//
//
//	return 0;
//}
//
//
//int GroupDriver::get_matrix_rows()
//{
//
//
//
//	return 0;
//}


// ===============================================================================================
// ===============================================================================================

} // namespace alchemist

