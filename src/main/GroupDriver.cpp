#include "GroupDriver.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

GroupDriver::GroupDriver(GroupID ID, Driver & _driver): ID(ID), driver(_driver), group(MPI_COMM_NULL), cl(SCALA), next_matrixID(1), next_libraryID(2) { }

GroupDriver::GroupDriver(GroupID ID, Driver & _driver, Log_ptr & _log): ID(ID), driver(_driver), group(MPI_COMM_NULL),
		log(_log), cl(SCALA), next_matrixID(1), next_libraryID(2) { }

GroupDriver::~GroupDriver() { }

void GroupDriver::start(tcp::socket socket)
{
//	log->info("DEBUG: GroupDriver : Start");

	session = std::make_shared<DriverSession>(std::move(socket), *this, ID, log);
	session->start();
}

void GroupDriver::idle_workers()
{
	alchemist_command command = _AM_IDLE;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);
}

void GroupDriver::open_workers()
{
	alchemist_command command = _AM_GROUP_OPEN_CONNECTIONS;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(group);
}

void GroupDriver::close_workers()
{
	alchemist_command command = _AM_GROUP_CLOSE_CONNECTIONS;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(group);
}

void GroupDriver::free_group()
{
	if (group != MPI_COMM_NULL) {
		close_workers();

		alchemist_command command = _AM_FREE_GROUP;

		log->info("Sending command {} to workers", get_command_name(command));

		MPI_Request req;
		MPI_Status status;
		MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
		MPI_Wait(&req, &status);

		MPI_Barrier(group);
		MPI_Comm_free(&group);
		group = MPI_COMM_NULL;
	}
}

void GroupDriver::print_info()
{
	alchemist_command command = _AM_PRINT_INFO;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	MPI_Barrier(group);
}

const map<WorkerID, WorkerInfo_ptr> & GroupDriver::allocate_workers(const uint16_t & num_requested_workers)
{
	driver.allocate_workers(ID, num_requested_workers);

	return workers;
}

vector<WorkerID> GroupDriver::deallocate_workers(const vector<WorkerID> & yielded_workers)
{
	return driver.deallocate_workers(ID, yielded_workers);
}

uint16_t GroupDriver::get_num_workers()
{
	return (uint16_t) workers.size();
}

uint64_t GroupDriver::get_num_rows(ArrayID & matrixID)
{
	return matrices[matrixID]->num_rows;
}

uint64_t GroupDriver::get_num_cols(ArrayID & matrixID)
{
	return matrices[matrixID]->num_cols;
}

void GroupDriver::set_group_comm(MPI_Comm & world, MPI_Group & temp_group)
{
	log->info("Creating new group");
	MPI_Comm_create_group(world, temp_group, 0, &group);
	MPI_Barrier(group);

}

void GroupDriver::ready_group()
{
	print_info();

	open_workers();
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

vector<WorkerInfo_ptr> GroupDriver::get_all_workers()
{
	return driver.get_all_workers();
}

vector<WorkerInfo_ptr> GroupDriver::get_active_workers()
{
	return driver.get_active_workers();
}

vector<WorkerInfo_ptr> GroupDriver::get_inactive_workers()
{
	return driver.get_inactive_workers();
}

vector<WorkerInfo_ptr> GroupDriver::get_assigned_workers()
{
	return driver.get_assigned_workers(ID);
}

LibraryID GroupDriver::load_library(string library_name, string library_path)
{
	alchemist_command command = _AM_WORKER_LOAD_LIBRARY;

	log->info("Sending command {} to workers {} {} ", get_command_name(command), library_name, library_path);

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	uint16_t library_name_length = library_name.length();
	uint16_t library_path_length = library_path.length();

	char library_name_c[library_name_length+1];
	char library_path_c[library_path_length+1];

	strcpy(library_name_c, library_name.c_str());
	strcpy(library_path_c, library_path.c_str());

	uint16_t library_name_c_length = strlen(library_name_c);
	uint16_t library_path_c_length = strlen(library_path_c);

	next_libraryID++;

	MPI_Bcast(&next_libraryID, 1, MPI_BYTE, 0, group);
	MPI_Bcast(&library_name_c_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(library_name_c, library_name_length+1, MPI_CHAR, 0, group);
	MPI_Bcast(&library_path_c_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(library_path_c, library_path_length+1, MPI_CHAR, 0, group);

	MPI_Barrier(group);

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

	libraries.insert(std::make_pair(next_libraryID, library_ptr));

	library_ptr->load();

	delete dlsym_error;

	MPI_Barrier(group);

	return next_libraryID;
}

void GroupDriver::add_worker(const WorkerID & workerID, const WorkerInfo_ptr & info)
{
	workers.insert(std::make_pair(workerID, info));
}

void GroupDriver::remove_worker(const WorkerID & workerID)
{
	workers.erase(workers.find(workerID));
}

void GroupDriver::run_task(Message & in_msg, Message & out_msg)
{
	alchemist_command command = _AM_WORKER_RUN_TASK;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	uint32_t in_data_length = in_msg.get_body_length();

	MPI_Bcast(&in_data_length, 1, MPI_UNSIGNED, 0, group);
	MPI_Bcast(in_msg.body(), in_data_length, MPI_CHAR, 0, group);

	MPI_Barrier(group);

	Parameters in, out;

	MPI_Barrier(group);

	LibraryID libID = in_msg.read_LibraryID();

	if (check_libraryID(libID)) {
		string function_name = in_msg.read_string();

		deserialize_parameters(in, in_msg);

		libraries[libID]->run(function_name, in, out);

		MPI_Barrier(group);

		int num_distmatrices;
		string distmatrix_name;
		uint16_t dmnl;
		uint64_t num_rows, num_cols;

		WorkerID primary_worker = workers.begin()->first;

		MPI_Recv(&num_distmatrices, 1, MPI_INT, primary_worker, 0, group, &status);

		if (num_distmatrices > 0) {
			ArrayID matrixIDs[num_distmatrices];
			for (int i = 0; i < num_distmatrices; i++) {
				MPI_Recv(&dmnl, 1, MPI_UNSIGNED_SHORT, primary_worker, 0, group, &status);
				char distmatrix_name_c[dmnl];
				MPI_Recv(distmatrix_name_c, dmnl, MPI_CHAR, primary_worker, 0, group, &status);
				distmatrix_name = string(distmatrix_name_c);

				MPI_Recv(&num_rows, 1, MPI_UNSIGNED_LONG, primary_worker, 0, group, &status);
				MPI_Recv(&num_cols, 1, MPI_UNSIGNED_LONG, primary_worker, 0, group, &status);

				bool sparse = false;
				uint8_t layout = 0;
				uint8_t num_partitions = (uint8_t) workers.size();

				matrices.insert(std::make_pair(next_matrixID, std::make_shared<ArrayInfo>(next_matrixID, distmatrix_name, num_rows, num_cols, sparse, layout, num_partitions)));
				matrixIDs[i] = next_matrixID++;
			}

			MPI_Bcast(&matrixIDs, num_distmatrices, MPI_UNSIGNED_SHORT, 0, group);

			uint64_t worker_num_rows;
			uint64_t * row_indices;

			for (int i = 0; i < num_distmatrices; i++) {

				std::clock_t start;

				MPI_Bcast(&matrixIDs[i], 1, MPI_UNSIGNED_SHORT, 0, group);
				MPI_Barrier(group);

				for (auto it = workers.begin(); it != workers.end(); it++) {

					WorkerID id = it->first;
					MPI_Recv(&worker_num_rows, 1, MPI_UNSIGNED_LONG, id, 0, group, &status);

					row_indices = new uint64_t[worker_num_rows];

					MPI_Recv(row_indices, (int) worker_num_rows, MPI_UNSIGNED_LONG, id, 0, group, &status);
					for (uint64_t j = 0; j < worker_num_rows; j++)
						matrices[matrixIDs[i]]->worker_assignments[row_indices[j]] = id;

					delete [] row_indices;
				}
//				out.write_ArrayInfo(matrices[matrixIDs[i]]->name, matrices[matrixIDs[i]]);
			}
		}

		MPI_Barrier(group);

		serialize_parameters(out, out_msg);
	}

//	out_msg.update_body_length();
//	out_msg.update_datatype_count();
}

void GroupDriver::deserialize_parameters(Parameters & p, Message & msg) {

	string name = "";
	datatype dt = NONE;
	uint32_t data_length;
	while (!msg.eom()) {
		dt = (datatype) msg.get_datatype();
		if (dt == PARAMETER) {
			name = msg.read_string();
			dt = (datatype) msg.preview_datatype();

			switch (dt) {
			case BYTE:
				p.add_byte(name, msg.read_byte());
				break;
			case CHAR:
				p.add_char(name, msg.read_char());
				break;
			case INT8:
				p.add_int8(name, msg.read_int8());
				break;
			case INT16:
				p.add_int16(name, msg.read_int16());
				break;
			case INT32:
				p.add_int32(name, msg.read_int32());
				break;
			case INT64:
				p.add_int64(name, msg.read_int64());
				break;
			case UINT8:
				p.add_uint8(name, msg.read_uint8());
				break;
			case UINT16:
				p.add_uint16(name, msg.read_uint16());
				break;
			case UINT32:
				p.add_uint32(name, msg.read_uint32());
				break;
			case UINT64:
				p.add_uint64(name, msg.read_uint64());
				break;
			case FLOAT:
				p.add_float(name, msg.read_float());
				break;
			case DOUBLE:
				p.add_double(name, msg.read_double());
				break;
			case STRING:
				p.add_string(name, msg.read_string());
				break;
			case ARRAY_ID:
				p.add_matrix_info(name, matrices[msg.read_ArrayID()]);
				break;
			}
		}
	}
}

void GroupDriver::serialize_parameters(Parameters & p, Message & msg)
{
	string name = "";
	datatype dt = p.get_next_parameter();
	while (dt != NONE) {
		msg.write_Parameter();
		name = p.get_name();
		log->info("RR {}", name);
		msg.write_string(name);

		switch (dt) {
		case BYTE:
			msg.write_byte(p.get_byte(name));
			break;
		case CHAR:
			msg.write_char(p.get_char(name));
			break;
		case INT8:
			msg.write_int8(p.get_int8(name));
			break;
		case INT16:
			msg.write_int16(p.get_int16(name));
			break;
		case INT32:
			msg.write_int32(p.get_int32(name));
			break;
		case INT64:
			msg.write_int64(p.get_int64(name));
			break;
		case UINT8:
			msg.write_uint8(p.get_uint8(name));
			break;
		case UINT16:
			msg.write_uint16(p.get_uint16(name));
			break;
		case UINT32:
			msg.write_uint32(p.get_uint32(name));
			break;
		case UINT64:
			msg.write_uint64(p.get_uint64(name));
			break;
		case FLOAT:
			msg.write_float(p.get_float(name));
			break;
		case DOUBLE:
			msg.write_double(p.get_double(name));
			break;
		case STRING:
			msg.write_string(p.get_string(name));
			break;
		case ARRAY_ID:
			msg.write_ArrayID(p.get_matrix_info(name)->ID);
			break;
		case ARRAY_INFO:
			msg.write_ArrayInfo(p.get_matrix_info(name));
			break;
		}

		dt = p.get_next_parameter();
	}

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
//		case MATRIX_INFO:
//			msg.write_matrix_info(*p.get_matrix_info(name));
//			break;
//		}
//
//		dt = p.get_next_parameter();
//	}
}

ArrayID GroupDriver::new_matrix(const ArrayInfo_ptr x)
{
	alchemist_command command = _AM_NEW_MATRIX;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	ArrayID matrixID = next_matrixID++;

	MPI_Bcast(&x->ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&x->num_rows, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&x->num_cols, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&x->sparse, 1, MPI_UNSIGNED_CHAR, 0, group);
	MPI_Bcast(&x->layout, 1, MPI_UNSIGNED_CHAR, 0, group);
	x->num_partitions = (uint8_t) workers.size();

	MPI_Barrier(group);

	matrices.insert(std::make_pair(matrixID, x));

	MPI_Barrier(group);

	determine_row_assignments(matrixID);

	return matrixID;
}

WorkerID * GroupDriver::get_row_assignments(ArrayID & matrixID)
{
	return matrices[matrixID]->worker_assignments;
}

bool GroupDriver::check_libraryID(LibraryID & libID)
{
	return true;
}

string GroupDriver::list_workers()
{
	return driver.list_all_workers();
}

ArrayInfo_ptr GroupDriver::get_matrix_info(const ArrayID matrixID)
{
	return matrices[matrixID];
}

void GroupDriver::determine_row_assignments(ArrayID & matrixID)
{
	uint64_t worker_num_rows;
	uint64_t * row_indices;

	std::clock_t start;

//	alchemist_command command = _AM_CLIENT_MATRIX_LAYOUT;
//
//	log->info("Sending command {} to workers", get_command_name(command));
//
	MPI_Request req;
	MPI_Status status;
//	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
//	MPI_Wait(&req, &status);

	MPI_Bcast(&matrices[matrixID]->ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Barrier(group);

	for (auto it = workers.begin(); it != workers.end(); it++) {
		WorkerID id = it->first;
		MPI_Recv(&worker_num_rows, 1, MPI_UNSIGNED_LONG, id, 0, group, &status);

		row_indices = new uint64_t[worker_num_rows];

		MPI_Recv(row_indices, (int) worker_num_rows, MPI_UNSIGNED_LONG, id, 0, group, &status);
		for (uint64_t i = 0; i < worker_num_rows; i++) {
			matrices[matrixID]->worker_assignments[row_indices[i]] = id;
		}

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

	return layout_rr;
}


// ----------------------------------------   File I/O   ----------------------------------------

int GroupDriver::read_HDF5() {

	log->info(string("GroupDriver::read_HDF5 not yet implemented"));



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


//ArrayHandle Driver::register_matrix(size_t num_rows, size_t num_cols) {
//
//	ArrayHandle handle{next_matrixID++};
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

