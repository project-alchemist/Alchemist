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

GroupWorker::GroupWorker(Group_ID _group_ID, Worker & _worker, io_context & _io_context, const unsigned int port, bool _primary_group_worker, Log_ptr & _log) :
    	    GroupWorker(_group_ID, _worker, _io_context, tcp::endpoint(tcp::v4(), port), _primary_group_worker, _log) { }

GroupWorker::GroupWorker(Group_ID _group_ID, Worker & _worker, io_context & _io_context, const tcp::endpoint & endpoint, bool _primary_group_worker, Log_ptr & _log) :
			Server(_io_context, endpoint, _log), grid(nullptr), current_grid(-1), group_ID(_group_ID), group(MPI_COMM_NULL), group_peers(MPI_COMM_NULL), worker(_worker),
			next_session_ID(0), current_matrix_ID(0), connection_open(false), primary_group_worker(_primary_group_worker)
{
	worker_ID = worker.get_ID();

	Server::set_log(_log);
}

GroupWorker::~GroupWorker() { }

void GroupWorker::set_group_comm(MPI_Comm & world, MPI_Group & temp_group)
{
	MPI_Comm_create_group(world, temp_group, 0, &group);
	MPI_Barrier(group);
}

void GroupWorker::set_group_peers_comm(MPI_Comm & world, MPI_Group & temp_group)
{
	MPI_Comm_create_group(world, temp_group, 0, &group_peers);

//
//	MPI_Barrier(group_peers);
//	if (grid == nullptr)
//	else {
//		grid = nullptr;
//		grid.reset(new El::Grid(El::mpi::Comm(group_peers)));
//	}
	grid = std::make_shared<El::Grid>(El::mpi::Comm(group_peers));
	MPI_Barrier(group_peers);

//	current_grid++;
//	grids.push_back(std::make_shared<El::Grid>(El::mpi::Comm(group_peers)));
//	layout_matrices();
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
		c = _AM_IDLE;

//		log->info("Number of threads: {}", threads.size());
	}

	for (auto & t: threads) t.join();

	return 0;
}

void GroupWorker::handle_free_group()
{
	if (group != MPI_COMM_NULL) {
		MPI_Barrier(group);

		MPI_Comm_free(&group);
		group = MPI_COMM_NULL;
	}

	if (group_peers != MPI_COMM_NULL) {
//		grids[current_grid] = nullptr;
		MPI_Barrier(group_peers);

		MPI_Comm_free(&group_peers);
		group_peers = MPI_COMM_NULL;
	}
}

int GroupWorker::handle_command(alchemist_command c)
{

	switch (c) {
		case _AM_IDLE:
			break;
//		case NEW_SESSION:
//			new_session();
//			break;
//		case END_SESSION:
//			end_session();
//			break;
		case _AM_PRINT_INFO:
			handle_print_info();
			break;
		case _AM_GROUP_OPEN_CONNECTIONS:
			handle_group_open_connections();
			break;
		case _AM_GROUP_CLOSE_CONNECTIONS:
			handle_group_close_connections();
			break;
		case _AM_NEW_MATRIX:
			new_matrix();
			break;
		case _AM_FREE_GROUP:
			handle_free_group();
			break;
		case _AM_CLIENT_MATRIX_LAYOUT:
			get_matrix_layout();
			break;
		case _AM_WORKER_LOAD_LIBRARY:
			load_library();
			break;
		case _AM_WORKER_RUN_TASK:
			run_task();
			break;
	}

	return 0;
}

void GroupWorker::handle_print_info()
{
	worker.print_info();
	MPI_Barrier(group);
}

void GroupWorker::handle_group_open_connections()
{
	connection_open = true;
	MPI_Barrier(group);
	accept_connection();
}

void GroupWorker::handle_group_close_connections()
{
	log->info("Closing connection");
	connection_open = false;

	MPI_Barrier(group);
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
	Library_ID library_ID;
	uint16_t library_name_length = 0;
	uint16_t library_path_length = 0;

	MPI_Bcast(&library_ID, 1, MPI_BYTE, 0, group);

	MPI_Bcast(&library_name_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	char library_name_c[library_name_length+1];
	MPI_Bcast(library_name_c, library_name_length+1, MPI_CHAR, 0, group);

	MPI_Bcast(&library_path_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	char library_path_c[library_path_length+1];
	MPI_Bcast(library_path_c, library_path_length+1, MPI_CHAR, 0, group);

	MPI_Barrier(group);

	string library_name = string(library_name_c);
	string library_path = string(library_path_c);

	char cstr[library_path.length()+1];
	std::strcpy(cstr, library_path.c_str());

	log->info("Loading library {} located at {}", library_name, library_path);

	void * lib = dlopen(library_path.c_str(), RTLD_GLOBAL);
	log->info("L 1");
	const char * dlopen_error = dlerror();
	log->info("L 2");
	if (dlopen_error != NULL) {
		log->info("L 3");
		log->info("dlopen failed: {}", string(dlopen_error));
		log->info("L 4");

		return 0;
	}

	log->info("L 6");
	dlerror();			// Reset errors

//	void * create_library = dlsym(lib, "create");

	log->info("L 7");
	create_t * create_library = reinterpret_cast<create_t *>(dlsym(lib, "create_library"));
	log->info("L 8");
	const char * dlsym_error = dlerror();
	log->info("L 9");
	if (dlsym_error != NULL) {
		log->info("L 10");
		log->info("dlsym with command \"create\" failed: {}", string(dlsym_error));
		log->info("L 11");

		return 0;
	}

//	Library * library = reinterpret_cast<Library*>(create_library(group));
	Library * library_ptr = reinterpret_cast<Library*>(create_library(group));

	libraries.insert(std::make_pair(library_ID, library_ptr));
//	Library_ptr library_ptr = std::make_shared<Library>(reinterpret_cast<Library*>(create_library(group)));
//
//	libraries.insert(std::make_pair(next_library_ID, library_ptr));

//	string task_name = "greet";
//	Parameters in, out;

	library_ptr->load();
//	library_ptr->run(task_name, in, out);
//
//	void * lib = dlopen(library_path.c_str(), RTLD_NOW);
//	if (lib == NULL) {
//		log->info("dlopen failed: {}", dlerror());
//
//		return -1;
//	}
//
//	dlerror();			// Reset errors
//
//	create_t * create_library = (create_t*) dlsym(lib, "create");
//	const char * dlsym_error = dlerror();
//	if (dlsym_error) {
////    		log->info("dlsym with command \"load\" failed: {}", dlerror());
//
////        	delete [] cstr;
////        	delete dlsym_error;
//
//		return -1;
//	}


//
//	void (*register_function)(void(*)());
//	void *handle = dlopen("libmylib.so");
//
//	register_function = dlsym(handle, "register_function");
//
//	register_function(in_main_func);


//	library = create_library(group);

//    	libraries.insert(std::make_pair(library_name, LibraryInfo(library_name, library_path, lib, library)));

//	if (!library->load()) {
//		log->info("Library {} loaded", library_name);
//
//		Parameters input;
//		Parameters output;
//
//		library->run("greet", input, output);
//	}

//	delete [] cstr;
	delete dlsym_error;

	MPI_Barrier(group);

	return 0;
}

int GroupWorker::new_matrix()
{
	log->info("Creating new Elemental distributed matrix");

	uint64_t num_rows, num_cols;
	unsigned char sparse, layout;

	MPI_Bcast(&current_matrix_ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&num_rows, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&num_cols, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&sparse, 1, MPI_UNSIGNED_CHAR, 0, group);
	MPI_Bcast(&layout, 1, MPI_UNSIGNED_CHAR, 0, group);

	MPI_Barrier(group);

	DistMatrix_ptr M = std::make_shared<El::DistMatrix<double, El::VR, El::STAR>>(num_rows, num_cols, *grid);
	El::Zero(*M);

	matrices.insert(std::make_pair(current_matrix_ID, M));
	log->info("{} Created new Elemental {}x{} distributed matrix {}", client_preamble(), num_rows, num_cols, current_matrix_ID);

	MPI_Barrier(group);

	get_matrix_layout();

	return 0;
}

void GroupWorker::read_matrix_parameters(Parameters & output_parameters)
{
	DistMatrix_ptr distmatrix_ptr = nullptr;
	std::vector<string> distmatrix_names;
	std::vector<DistMatrix_ptr> distmatrix_ptrs;
	string distmatrix_name = "";

	output_parameters.get_next_distmatrix(distmatrix_name, distmatrix_ptr);

	while (distmatrix_ptr != nullptr) {
		distmatrix_names.push_back(distmatrix_name);
		distmatrix_ptrs.push_back(distmatrix_ptr);

		output_parameters.get_next_distmatrix(distmatrix_name, distmatrix_ptr);
	}

	int num_distmatrices = (int) distmatrix_ptrs.size();

	if (primary_group_worker) MPI_Send(&num_distmatrices, 1, MPI_INT, 0, 0, group);

	if (num_distmatrices > 0) {

		if (primary_group_worker) {
			for (int i = 0; i < num_distmatrices; i++) {
				uint16_t dmnl = (uint16_t) distmatrix_names[i].length()+1;

				MPI_Send(&dmnl, 1, MPI_UNSIGNED_SHORT, 0, 0, group);
				MPI_Send(distmatrix_names[i].c_str(), dmnl, MPI_CHAR, 0, 0, group);

				uint64_t num_rows = (uint64_t) distmatrix_ptrs[i]->Height();
				uint64_t num_cols = (uint64_t) distmatrix_ptrs[i]->Width();

				MPI_Send(&num_rows, 1, MPI_UNSIGNED_LONG, 0, 0, group);
				MPI_Send(&num_cols, 1, MPI_UNSIGNED_LONG, 0, 0, group);
			}
		}

		Matrix_ID matrix_IDs[num_distmatrices];
		MPI_Bcast(&matrix_IDs, num_distmatrices, MPI_UNSIGNED_SHORT, 0, group);

		for (int i = 0; i < num_distmatrices; i++) {
			matrices.insert(std::make_pair(matrix_IDs[i], distmatrix_ptrs[i]));

			log->info("Creating vector of local rows for matrix {}", distmatrix_names[i]);

			MPI_Bcast(&matrix_IDs[i], 1, MPI_UNSIGNED_SHORT, 0, group);
			MPI_Barrier(group);

			distmatrix_ptr = matrices[matrix_IDs[i]];
			uint64_t num_local_rows = (uint64_t) distmatrix_ptr->LocalHeight();
			uint64_t * local_rows = new uint64_t[num_local_rows];

			for (uint64_t i = 0; i < num_local_rows; i++) local_rows[i] = (uint64_t) distmatrix_ptr->GlobalRow(i);

			MPI_Send(&num_local_rows, 1, MPI_UNSIGNED_LONG, 0, 0, group);
			MPI_Send(local_rows, (int) num_local_rows, MPI_UNSIGNED_LONG, 0, 0, group);

			delete [] local_rows;
		}
	}

	MPI_Barrier(group);
}

int GroupWorker::get_matrix_layout()
{
//	std::clock_t start = std::clock();

	log->info("Creating vector of local rows");

	Matrix_ID ID;

	MPI_Bcast(&ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Barrier(group);

	DistMatrix_ptr matrix = matrices[ID];
	uint64_t num_local_rows = (uint64_t) matrix->LocalHeight();
	uint64_t * local_rows = new uint64_t[num_local_rows];

	for (uint64_t i = 0; i < num_local_rows; i++) local_rows[i] = (uint64_t) matrix->GlobalRow(i);

	MPI_Send(&num_local_rows, 1, MPI_UNSIGNED_LONG, 0, 0, group);
	MPI_Send(local_rows, (int) num_local_rows, MPI_UNSIGNED_LONG, 0, 0, group);

	delete [] local_rows;

	MPI_Barrier(group);

	return 0;
}

void GroupWorker::set_value(Matrix_ID ID, uint64_t row, uint64_t col, float value)
{
	matrices[ID]->Set(row, col, value);
}

void GroupWorker::set_value(Matrix_ID ID, uint64_t row, uint64_t col, double value)
{
	matrices[ID]->Set(row, col, value);
}

void GroupWorker::get_value(Matrix_ID ID, uint64_t row, uint64_t col, float & value)
{
	value = matrices[ID]->Get(row, col);
}

void GroupWorker::get_value(Matrix_ID ID, uint64_t row, uint64_t col, double & value)
{
	value = matrices[ID]->Get(row, col);
}

void GroupWorker::print_data(Matrix_ID ID)
{
	std::stringstream ss;
	ss << "LOCAL DATA:" << std::endl;
	ss << "Local size: " << matrices[ID]->LocalHeight() << " x " << matrices[ID]->LocalWidth() << std::endl;
	for (El::Int i = 0; i < matrices[ID]->LocalHeight(); i++) {
		for (El::Int j = 0; j < matrices[ID]->LocalWidth(); j++)
			ss <<  matrices[ID]->GetLocal(i, j) << " ";
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

bool GroupWorker::check_library_ID(Library_ID & lib_ID)
{
	return true;
}

void GroupWorker::run_task()
{
	uint32_t data_length;
	MPI_Bcast(&data_length, 1, MPI_UNSIGNED, 0, group);

	char * data = new char[data_length];
	MPI_Bcast(data, data_length, MPI_CHAR, 0, group);

	MPI_Barrier(group);

	Message temp_in_msg(data_length), temp_out_msg(data_length);
	Parameters in, out;

	temp_in_msg.copy_body(&data[0], data_length);
	MPI_Barrier(group);

	Library_ID lib_ID = temp_in_msg.read_uint8();
	if (check_library_ID(lib_ID)) {
		string function_name = temp_in_msg.read_string();

		deserialize_parameters(in, temp_in_msg);

		libraries[lib_ID]->run(function_name, in, out);

		MPI_Barrier(group);

//		serialize_parameters(out, temp_out_msg);

		read_matrix_parameters(out);
	}
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

void GroupWorker::deserialize_parameters(Parameters & p, Message & msg)
{

	string name = "";
	datatype dt = NONE;
	uint32_t data_length;
	while (!msg.eom()) {
		dt = (datatype) msg.next_datatype();
		data_length = msg.next_data_length();
		if (dt == PARAMETER) {
			msg.read_parameter();
			name = msg.read_string();
			dt = (datatype) msg.next_datatype();
			data_length = msg.next_data_length();

			switch(dt) {
			case CHAR:
				p.add_char(name, msg.read_char());
				break;
			case SIGNED_CHAR:
				p.add_signed_char(name, msg.read_signed_char());
				break;
			case UNSIGNED_CHAR:
				p.add_unsigned_char(name, msg.read_unsigned_char());
				break;
			case CHARACTER:
				p.add_character(name, msg.read_char());
				break;
			case WCHAR:
				p.add_wchar(name, msg.read_wchar());
				break;
			case SHORT:
				p.add_short(name, msg.read_short());
				break;
			case UNSIGNED_SHORT:
				p.add_unsigned_short(name, msg.read_unsigned_short());
				break;
			case INT:
				p.add_int(name, msg.read_int());
				break;
			case UNSIGNED:
				p.add_unsigned(name, msg.read_unsigned());
				break;
			case LONG:
				p.add_long(name, msg.read_long());
				break;
			case UNSIGNED_LONG:
				p.add_unsigned_long(name, msg.read_unsigned_long());
				break;
			case LONG_LONG_INT:
				p.add_long_long_int(name, msg.read_long_long_int());
				break;
			case LONG_LONG:
				p.add_long_long(name, msg.read_long_long());
				break;
			case UNSIGNED_LONG_LONG:
				p.add_unsigned_long_long(name, msg.read_unsigned_long_long());
				break;
			case FLOAT:
				p.add_float(name, msg.read_float());
				break;
			case DOUBLE:
				p.add_double(name, msg.read_double());
				break;
	//				case LONG_DOUBLE:
	//					p.add_long_double(name, msg.read_long_double());
	//					break;
			case BYTE:
				p.add_byte(name, msg.read_byte());
				break;
			case BOOL:
				p.add_bool(name, msg.read_bool());
				break;
			case INTEGER:
				p.add_integer(name, msg.read_integer());
				break;
			case REAL:
				p.add_real(name, msg.read_real());
				break;
			case LOGICAL:
				p.add_logical(name, msg.read_logical());
				break;
	//				case COMPLEX:
	//					p.add_complex(name, msg.read_complex());
	//					break;
			case DOUBLE_PRECISION:
				p.add_double_precision(name, msg.read_double_precision());
				break;
			case REAL4:
				p.add_real4(name, msg.read_real4());
				break;
	//				case COMPLEX8:
	//					p.add_complex8(name, msg.read_complex8());
	//					break;
			case REAL8:
				p.add_real8(name, msg.read_real8());
				break;
	//				case COMPLEX16:
	//					p.add_complex16(name, msg.read_complex16());
	//					break;
			case INTEGER1:
				p.add_integer1(name, msg.read_integer1());
				break;
			case INTEGER2:
				p.add_integer2(name, msg.read_integer2());
				break;
			case INTEGER4:
				p.add_integer4(name, msg.read_integer4());
				break;
			case INTEGER8:
				p.add_integer8(name, msg.read_integer8());
				break;
			case INT8_T:
				p.add_int8(name, msg.read_int8());
				break;
			case INT16_T:
				p.add_int16(name, msg.read_int16());
				break;
			case INT32_T:
				p.add_int32(name, msg.read_int32());
				break;
			case INT64_T:
				p.add_int64(name, msg.read_int64());
				break;
			case UINT8_T:
				p.add_uint8(name, msg.read_uint8());
				break;
			case UINT16_T:
				p.add_uint16(name, msg.read_uint16());
				break;
			case UINT32_T:
				p.add_uint32(name, msg.read_uint32());
				break;
			case UINT64_T:
				p.add_uint64(name, msg.read_uint64());
				break;
	//				case FLOAT_INT:
	//					p.add_float_int(name, msg.read_float_int());
	//					break;
	//				case DOUBLE_INT:
	//					p.add_double_int(name, msg.read_double_int());
	//					break;
	//				case LONG_INT:
	//					p.add_long_int(name, msg.read_long_int());
	//					break;
	//				case SHORT_INT:
	//					p.add_short_int(name, msg.read_short_int());
	//					break;
	//				case LONG_DOUBLE_INT:
	//					p.add_long_double_int(name, msg.read_long_double_int());
	//					break;
			case STRING:
				p.add_string(name, msg.read_string());
				break;
			case WSTRING:
				p.add_wstring(name, msg.read_wstring());
				break;
			case MATRIX_ID:
				auto iid = msg.read_matrix_ID();
				p.add_distmatrix(name, matrices[iid]);
				break;
			}
		}
	}
}

void GroupWorker::serialize_parameters(Parameters & p, Message & msg) {

	string name = "";
	datatype dt = p.get_next_parameter();
	while (dt != NONE) {
		msg.add_parameter();
		name = p.get_name();
		msg.add_string(name);

		switch(dt) {
		case CHAR:
			msg.add_char(p.get_char(name));
			break;
		case SIGNED_CHAR:
//			msg.add_signed_char(p.get_signed_char(name));
			break;
		case UNSIGNED_CHAR:
//			unsigned char value = p.get_unsigned_char(name);
//			msg.add_unsigned_char();
			break;
		case CHARACTER:
//			msg.add_character(p.get_char(name));
			break;
		case WCHAR:
//			msg.add_wchar(p.read_wchar(name));
			break;
		case SHORT:
//			msg.add_short(p.read_short(name));
			break;
		case UNSIGNED_SHORT:
//			msg.add_unsigned_short(p.read_unsigned_short(name));
			break;
		case INT:
			msg.add_int(p.get_int(name));
			break;
		case UNSIGNED:
//			msg.add_unsigned(p.get_unsigned(name));
			break;
		case LONG:
//			msg.add_long(p.get_long(name));
			break;
		case UNSIGNED_LONG:
//			msg.add_unsigned_long(p.get_unsigned_long(name));
			break;
		case LONG_LONG_INT:
//			msg.add_long_long_int(p.get_long_long_int(name));
			break;
		case LONG_LONG:
//			msg.add_long_long(p.get_long_long(name));
			break;
		case UNSIGNED_LONG_LONG:
//			msg.add_unsigned_long_long(p.get_unsigned_long_long(name));
			break;
		case FLOAT:
			msg.add_float(p.get_float(name));
			break;
		case DOUBLE:
			msg.add_double(p.get_double(name));
			break;
//				case LONG_DOUBLE:
//					p.add_long_double(name, msg.read_long_double());
//					break;
		case BYTE:
			msg.add_byte(p.get_byte(name));
			break;
		case BOOL:
			msg.add_bool(p.get_bool(name));
			break;
		case INTEGER:
//			msg.add_integer(p.get_integer(name));
			break;
		case REAL:
//			msg.add_real(p.get_real(name));
			break;
		case LOGICAL:
			msg.add_logical(p.get_logical(name));
			break;
//				case COMPLEX:
//					p.add_complex(name, msg.read_complex());
//					break;
		case DOUBLE_PRECISION:
//			msg.add_double_precision(p.get_double_precision(name));
			break;
		case REAL4:
//			msg.add_real4(p.get_real4(name));
			break;
//				case COMPLEX8:
//					p.add_complex8(name, msg.read_complex8());
//					break;
		case REAL8:
//			msg.add_real8(p.get_real8(name));
			break;
//				case COMPLEX16:
//					p.add_complex16(name, msg.read_complex16());
//					break;
		case INTEGER1:
			msg.add_integer1(p.get_integer1(name));
			break;
		case INTEGER2:
			msg.add_integer2(p.get_integer2(name));
			break;
		case INTEGER4:
			msg.add_integer4(p.get_integer4(name));
			break;
		case INTEGER8:
			msg.add_integer8(p.get_integer8(name));
			break;
		case INT8_T:
			msg.add_int8(p.get_int8(name));
			break;
		case INT16_T:
			msg.add_int16(p.get_int16(name));
			break;
		case INT32_T:
			msg.add_int32(p.get_int32(name));
			break;
		case INT64_T:
			msg.add_int64(p.get_int64(name));
			break;
		case UINT8_T:
			msg.add_uint8(p.get_uint8(name));
			break;
		case UINT16_T:
			msg.add_uint16(p.get_uint16(name));
			break;
		case UINT32_T:
			msg.add_uint32(p.get_uint32(name));
			break;
		case UINT64_T:
			msg.add_uint64(p.get_uint64(name));
			break;
//				case FLOAT_INT:
//					p.add_float_int(name, msg.read_float_int());
//					break;
//				case DOUBLE_INT:
//					p.add_double_int(name, msg.read_double_int());
//					break;
//				case LONG_INT:
//					p.add_long_int(name, msg.read_long_int());
//					break;
//				case SHORT_INT:
//					p.add_short_int(name, msg.read_short_int());
//					break;
//				case LONG_DOUBLE_INT:
//					p.add_long_double_int(name, msg.read_long_double_int());
//					break;
		case STRING:
			msg.add_string(p.get_string(name));
			break;
		case WSTRING:
//			msg.add_wstring(p.get_wstring(name));
			break;
		case MATRIX_ID:
			msg.add_matrix_ID(p.get_matrix_info(name)->ID);
			break;
		}

		dt = p.get_next_parameter();
	}
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
	if (connection_open) {
		acceptor.async_accept(
			[this](error_code ec, tcp::socket socket)
			{
	//			if (!ec) std::make_shared<WorkerSession>(std::move(socket), *this, next_session_ID++, log)->start();
				if (!ec) new_session(std::move(socket));

				accept_connection();
			});

		ic.run();
	}

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
