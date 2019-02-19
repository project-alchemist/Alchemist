#include "GroupWorker.hpp"

namespace alchemist {

//GroupWorker::GroupWorker(GroupID _groupID, WorkerID _workerID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//		const unsigned int port, Log_ptr & _log) :
//    	    GroupWorker(_groupID, _workerID, _group, _group_peers, _io_context, tcp::endpoint(tcp::v4(), port), _log) { }
//
//GroupWorker::GroupWorker(GroupID _groupID, WorkerID _workerID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//		const tcp::endpoint & endpoint, Log_ptr & _log) :
//			Server(_io_context, endpoint, _log), groupID(_groupID), workerID(_workerID), group(_group), group_peers(_group_peers),
//			next_sessionID(0), accept_connections(false)

GroupWorker::GroupWorker(GroupID _groupID, Worker & _worker, io_context & _io_context, const unsigned int port, bool _primary_group_worker, Log_ptr & _log) :
    	    GroupWorker(_groupID, _worker, _io_context, tcp::endpoint(tcp::v4(), port), _primary_group_worker, _log) { }

GroupWorker::GroupWorker(GroupID _groupID, Worker & _worker, io_context & _io_context, const tcp::endpoint & endpoint, bool _primary_group_worker, Log_ptr & _log) :
			Server(_io_context, endpoint, _log), grid(nullptr), current_grid(-1), groupID(_groupID), group(MPI_COMM_NULL), group_peers(MPI_COMM_NULL), worker(_worker),
			next_sessionID(0), current_matrixID(0), connection_open(false), primary_group_worker(_primary_group_worker)
{
	workerID = worker.get_ID();

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

WorkerID GroupWorker::get_workerID()
{
	return workerID;
}

GroupID GroupWorker::get_groupID()
{
	return workerID;
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
//	MPI_Recv(&clientID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//
//	uint16_t num_peers = 0;
//
//	MPI_Recv(&num_peers, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//
//	uint16_t peerID;
//	const int groupIDs[num_peers+1];
//	const int group_peerIDs[num_peers];
//
//	groupIDs[0] = 0;			// First member is the driver
//	for (uint16_t i = 0; i < num_peers; i++) {
//		MPI_Recv(&peerID, 1, MPI_UNSIGNED_SHORT, 0, ID, world, &status);
//		groupIDs[i+1] = peerID;
//		group_peerIDs[i] = peerID;
//	}
//
//	MPI_Group world_group;
//	MPI_Group temp_group;
//
//	MPI_Comm_group(world, &world_group);
//
//	MPI_Group_incl(world_group, (int) (num_peers+1), groupIDs, &temp_group);
//	MPI_Comm_create_group(world, temp_group, 0, &group);			// Create a new communicator based on the groupIDs
//
//	MPI_Group_incl(world_group, (int) num_peers, group_peerIDs, &temp_group);
//	MPI_Comm_create_group(world, temp_group, 0, &group_peers);		// Create a new communicator based on the group_peerIDs
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
	ss << "This is a test string from Alchemist worker " << workerID;

	return ss.str();
}


//string GroupWorker::preamble()
//{
//	return client_preamble() + " " + session_preamble();
//}

string GroupWorker::client_preamble()
{
	std::stringstream ss;

	ss << "[Job " << groupID << " (" << address.c_str() << ":" << port << ")]";

	return ss.str();
}
//
//string GroupWorker::session_preamble()
//{
//	std::stringstream ss;
//
//	ss << "[Session " << sessionID << "]";
//
//	return ss.str();
//}

int GroupWorker::load_library()
{
	LibraryID libraryID;
	uint16_t library_name_length = 0;
	uint16_t library_path_length = 0;

	MPI_Bcast(&libraryID, 1, MPI_BYTE, 0, group);

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

	void * lib = dlopen(library_path.c_str(), RTLD_LAZY);
	const char * dlopen_error = dlerror();
	if (dlopen_error != NULL) {
		log->info("dlopen failed: {}", string(dlopen_error));

		return 0;
	}

	dlerror();			// Reset errors

	create_t * create_library = reinterpret_cast<create_t *>(dlsym(lib, "create_library"));
	const char * dlsym_error = dlerror();
	if (dlsym_error != NULL) {
		log->info("dlsym with command \"create\" failed: {}", string(dlsym_error));

		return 0;
	}

	Library * library_ptr = reinterpret_cast<Library*>(create_library(group));

	libraries.insert(std::make_pair(libraryID, library_ptr));

	library_ptr->load();

	delete dlsym_error;

	MPI_Barrier(group);

	return 0;
}

int GroupWorker::new_matrix()
{
	log->info("Creating new Elemental distributed matrix");

	uint64_t num_rows, num_cols;
	unsigned char sparse, layout;

	MPI_Bcast(&current_matrixID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&num_rows, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&num_cols, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&sparse, 1, MPI_UNSIGNED_CHAR, 0, group);
	MPI_Bcast(&layout, 1, MPI_UNSIGNED_CHAR, 0, group);

	MPI_Barrier(group);

	DistMatrix_ptr M = std::make_shared<El::DistMatrix<double, El::VR, El::STAR>>(num_rows, num_cols, *grid);
	El::Zero(*M);

	matrices.insert(std::make_pair(current_matrixID, M));
	log->info("{} Created new Elemental {}x{} distributed matrix {}", client_preamble(), num_rows, num_cols, current_matrixID);

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

		ArrayID matrixIDs[num_distmatrices];
		MPI_Bcast(&matrixIDs, num_distmatrices, MPI_UNSIGNED_SHORT, 0, group);

		for (int i = 0; i < num_distmatrices; i++) {
			matrices.insert(std::make_pair(matrixIDs[i], distmatrix_ptrs[i]));

			log->info("Creating vector of local rows for matrix {}", distmatrix_names[i]);

			MPI_Bcast(&matrixIDs[i], 1, MPI_UNSIGNED_SHORT, 0, group);
			MPI_Barrier(group);

			distmatrix_ptr = matrices[matrixIDs[i]];
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
	log->info("Creating vector of local rows");

	ArrayID ID;

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

void GroupWorker::set_value(ArrayID ID, uint64_t row, uint64_t col, float value)
{
	matrices[ID]->SetLocal(matrices[ID]->LocalRow(row), matrices[ID]->LocalCol(col), value);
}

void GroupWorker::set_value(ArrayID ID, uint64_t row, uint64_t col, double value)
{
	matrices[ID]->SetLocal(matrices[ID]->LocalRow(row), matrices[ID]->LocalCol(col), value);
}

void GroupWorker::get_value(ArrayID ID, uint64_t row, uint64_t col, float & value)
{
	value = matrices[ID]->GetLocal(matrices[ID]->LocalRow(row), matrices[ID]->LocalCol(col));
}

void GroupWorker::get_value(ArrayID ID, uint64_t row, uint64_t col, double & value)
{
//	clock_t start1 = clock();
	value = matrices[ID]->GetLocal(matrices[ID]->LocalRow(row), matrices[ID]->LocalCol(col));
}

void GroupWorker::print_data(ArrayID ID)
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

bool GroupWorker::check_libraryID(LibraryID & libID)
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

	LibraryID libID = temp_in_msg.read_uint8();
	if (check_libraryID(libID)) {
		string function_name = temp_in_msg.read_string();

		deserialize_parameters(in, temp_in_msg);

		libraries[libID]->run(function_name, in, out);

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
//	while (!msg.eom()) {
//		dt = (datatype) msg.next_datatype();
//		data_length = msg.next_data_length();
//		if (dt == PARAMETER) {
//			msg.read_parameter();
//			name = msg.read_string();
//			dt = (datatype) msg.next_datatype();
//			data_length = msg.next_data_length();
//
//			switch(dt) {
//			case CHAR:
//				p.write_char(name, msg.read_char());
//				break;
//			case SIGNED_CHAR:
//				p.write_signed_char(name, msg.read_signed_char());
//				break;
//			case UNSIGNED_CHAR:
//				p.write_unsigned_char(name, msg.read_unsigned_char());
//				break;
//			case CHARACTER:
//				p.write_character(name, msg.read_char());
//				break;
//			case WCHAR:
//				p.write_wchar(name, msg.read_wchar());
//				break;
//			case SHORT:
//				p.write_short(name, msg.read_short());
//				break;
//			case UNSIGNED_SHORT:
//				p.write_unsigned_short(name, msg.read_unsigned_short());
//				break;
//			case INT:
//				p.write_int(name, msg.read_int());
//				break;
//			case UNSIGNED:
//				p.write_unsigned(name, msg.read_unsigned());
//				break;
//			case LONG:
//				p.write_long(name, msg.read_long());
//				break;
//			case UNSIGNED_LONG:
//				p.write_unsigned_long(name, msg.read_unsigned_long());
//				break;
//			case LONG_LONG_INT:
//				p.write_long_long_int(name, msg.read_long_long_int());
//				break;
//			case LONG_LONG:
//				p.write_long_long(name, msg.read_long_long());
//				break;
//			case UNSIGNED_LONG_LONG:
//				p.write_unsigned_long_long(name, msg.read_unsigned_long_long());
//				break;
//			case FLOAT:
//				p.write_float(name, msg.read_float());
//				break;
//			case DOUBLE:
//				p.write_double(name, msg.read_double());
//				break;
//	//				case LONG_DOUBLE:
//	//					p.write_long_double(name, msg.read_long_double());
//	//					break;
//			case BYTE:
//				p.write_byte(name, msg.read_byte());
//				break;
//			case BOOL:
//				p.write_bool(name, msg.read_bool());
//				break;
//			case INTEGER:
//				p.write_integer(name, msg.read_integer());
//				break;
//			case REAL:
//				p.write_real(name, msg.read_real());
//				break;
//			case LOGICAL:
//				p.write_logical(name, msg.read_logical());
//				break;
//	//				case COMPLEX:
//	//					p.write_complex(name, msg.read_complex());
//	//					break;
//			case DOUBLE_PRECISION:
//				p.write_double_precision(name, msg.read_double_precision());
//				break;
//			case REAL4:
//				p.write_real4(name, msg.read_real4());
//				break;
//	//				case COMPLEX8:
//	//					p.write_complex8(name, msg.read_complex8());
//	//					break;
//			case REAL8:
//				p.write_real8(name, msg.read_real8());
//				break;
//	//				case COMPLEX16:
//	//					p.write_complex16(name, msg.read_complex16());
//	//					break;
//			case INTEGER1:
//				p.write_integer1(name, msg.read_integer1());
//				break;
//			case INTEGER2:
//				p.write_integer2(name, msg.read_integer2());
//				break;
//			case INTEGER4:
//				p.write_integer4(name, msg.read_integer4());
//				break;
//			case INTEGER8:
//				p.write_integer8(name, msg.read_integer8());
//				break;
//			case INT8_T:
//				p.write_int8(name, msg.read_int8());
//				break;
//			case INT16_T:
//				p.write_int16(name, msg.read_int16());
//				break;
//			case INT32_T:
//				p.write_int32(name, msg.read_int32());
//				break;
//			case INT64_T:
//				p.write_int64(name, msg.read_int64());
//				break;
//			case UINT8_T:
//				p.write_uint8(name, msg.read_uint8());
//				break;
//			case UINT16_T:
//				p.write_uint16(name, msg.read_uint16());
//				break;
//			case UINT32_T:
//				p.write_uint32(name, msg.read_uint32());
//				break;
//			case UINT64_T:
//				p.write_uint64(name, msg.read_uint64());
//				break;
//	//				case FLOAT_INT:
//	//					p.write_float_int(name, msg.read_float_int());
//	//					break;
//	//				case DOUBLE_INT:
//	//					p.write_double_int(name, msg.read_double_int());
//	//					break;
//	//				case LONG_INT:
//	//					p.write_long_int(name, msg.read_long_int());
//	//					break;
//	//				case SHORT_INT:
//	//					p.write_short_int(name, msg.read_short_int());
//	//					break;
//	//				case LONG_DOUBLE_INT:
//	//					p.write_long_double_int(name, msg.read_long_double_int());
//	//					break;
//			case STRING:
//				p.write_string(name, msg.read_string());
//				break;
//			case WSTRING:
//				p.write_wstring(name, msg.read_wstring());
//				break;
//			case MATRIXID:
//				auto iid = msg.read_matrixID();
//				p.write_distmatrix(name, matrices[iid]);
//				break;
//			}
//		}
//	}
}

void GroupWorker::serialize_parameters(Parameters & p, Message & msg)
{

	string name = "";
	datatype dt = p.get_next_parameter();
//	while (dt != NONE) {
//		msg.write_parameter();
//		name = p.get_name();
//		msg.write_string(name);
//
//		switch(dt) {
//		case CHAR:
//			msg.write_char(p.get_char(name));
//			break;
//		case SIGNED_CHAR:
////			msg.write_signed_char(p.get_signed_char(name));
//			break;
//		case UNSIGNED_CHAR:
////			unsigned char value = p.get_unsigned_char(name);
////			msg.write_unsigned_char();
//			break;
//		case CHARACTER:
////			msg.write_character(p.get_char(name));
//			break;
//		case WCHAR:
////			msg.write_wchar(p.read_wchar(name));
//			break;
//		case SHORT:
////			msg.write_short(p.read_short(name));
//			break;
//		case UNSIGNED_SHORT:
////			msg.write_unsigned_short(p.read_unsigned_short(name));
//			break;
//		case INT:
//			msg.write_int(p.get_int(name));
//			break;
//		case UNSIGNED:
////			msg.write_unsigned(p.get_unsigned(name));
//			break;
//		case LONG:
////			msg.write_long(p.get_long(name));
//			break;
//		case UNSIGNED_LONG:
////			msg.write_unsigned_long(p.get_unsigned_long(name));
//			break;
//		case LONG_LONG_INT:
////			msg.write_long_long_int(p.get_long_long_int(name));
//			break;
//		case LONG_LONG:
////			msg.write_long_long(p.get_long_long(name));
//			break;
//		case UNSIGNED_LONG_LONG:
////			msg.write_unsigned_long_long(p.get_unsigned_long_long(name));
//			break;
//		case FLOAT:
//			msg.write_float(p.get_float(name));
//			break;
//		case DOUBLE:
//			msg.write_double(p.get_double(name));
//			break;
////				case LONG_DOUBLE:
////					p.write_long_double(name, msg.read_long_double());
////					break;
//		case BYTE:
//			msg.write_byte(p.get_byte(name));
//			break;
//		case BOOL:
//			msg.write_bool(p.get_bool(name));
//			break;
//		case INTEGER:
////			msg.write_integer(p.get_integer(name));
//			break;
//		case REAL:
////			msg.write_real(p.get_real(name));
//			break;
//		case LOGICAL:
//			msg.write_logical(p.get_logical(name));
//			break;
////				case COMPLEX:
////					p.write_complex(name, msg.read_complex());
////					break;
//		case DOUBLE_PRECISION:
////			msg.write_double_precision(p.get_double_precision(name));
//			break;
//		case REAL4:
////			msg.write_real4(p.get_real4(name));
//			break;
////				case COMPLEX8:
////					p.write_complex8(name, msg.read_complex8());
////					break;
//		case REAL8:
////			msg.write_real8(p.get_real8(name));
//			break;
////				case COMPLEX16:
////					p.write_complex16(name, msg.read_complex16());
////					break;
//		case INTEGER1:
//			msg.write_integer1(p.get_integer1(name));
//			break;
//		case INTEGER2:
//			msg.write_integer2(p.get_integer2(name));
//			break;
//		case INTEGER4:
//			msg.write_integer4(p.get_integer4(name));
//			break;
//		case INTEGER8:
//			msg.write_integer8(p.get_integer8(name));
//			break;
//		case INT8_T:
//			msg.write_int8(p.get_int8(name));
//			break;
//		case INT16_T:
//			msg.write_int16(p.get_int16(name));
//			break;
//		case INT32_T:
//			msg.write_int32(p.get_int32(name));
//			break;
//		case INT64_T:
//			msg.write_int64(p.get_int64(name));
//			break;
//		case UINT8_T:
//			msg.write_uint8(p.get_uint8(name));
//			break;
//		case UINT16_T:
//			msg.write_uint16(p.get_uint16(name));
//			break;
//		case UINT32_T:
//			msg.write_uint32(p.get_uint32(name));
//			break;
//		case UINT64_T:
//			msg.write_uint64(p.get_uint64(name));
//			break;
////				case FLOAT_INT:
////					p.write_float_int(name, msg.read_float_int());
////					break;
////				case DOUBLE_INT:
////					p.write_double_int(name, msg.read_double_int());
////					break;
////				case LONG_INT:
////					p.write_long_int(name, msg.read_long_int());
////					break;
////				case SHORT_INT:
////					p.write_short_int(name, msg.read_short_int());
////					break;
////				case LONG_DOUBLE_INT:
////					p.write_long_double_int(name, msg.read_long_double_int());
////					break;
//		case STRING:
//			msg.write_string(p.get_string(name));
//			break;
//		case WSTRING:
////			msg.write_wstring(p.get_wstring(name));
//			break;
//		case MATRIXID:
//			msg.write_matrixID(p.get_matrix_info(name)->ID);
//			break;
//		}
//
//		dt = p.get_next_parameter();
//	}
}

int GroupWorker::new_session(tcp::socket socket)
{
	next_sessionID++;
	auto session_ptr = std::make_shared<WorkerSession>(std::move(socket), *this, next_sessionID, groupID, log);
	sessions.insert(std::make_pair(next_sessionID, session_ptr));
	sessions[next_sessionID]->start();

	return 0;
}

int GroupWorker::accept_connection()
{
	if (connection_open) {
		acceptor.async_accept(
			[this](error_code ec, tcp::socket socket)
			{
	//			if (!ec) std::make_shared<WorkerSession>(std::move(socket), *this, next_sessionID++, log)->start();
				if (!ec) new_session(std::move(socket));

				accept_connection();
			});

		ic.run();
	}

	return 0;
}

//int GroupWorker::start_new_session()
//{
//	next_sessionID++;
//	session = std::make_shared<WorkerSession>(std::move(socket), *this, next_sessionID, log);
//
//	return 0;
//}
//
//int GroupWorker::accept_connection()
//{
//	acceptor.async_accept(
//		[this](error_code ec, tcp::socket socket)
//		{
////			if (!ec) std::make_shared<GroupWorkerSession>(std::move(socket), *this, next_sessionID++, log)->start();
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
