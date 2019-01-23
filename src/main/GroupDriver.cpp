#include "GroupDriver.hpp"

namespace alchemist {

// ===============================================================================================
// =======================================   CONSTRUCTOR   =======================================

GroupDriver::GroupDriver(Group_ID _ID, Driver & _driver): ID(_ID), driver(_driver), group(MPI_COMM_NULL), cl(SCALA), next_matrix_ID(1), next_library_ID(2) { }

GroupDriver::GroupDriver(Group_ID _ID, Driver & _driver, Log_ptr & _log): ID(_ID), driver(_driver), group(MPI_COMM_NULL),
		log(_log), cl(SCALA), next_matrix_ID(1), next_library_ID(2) { }

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

const map<Worker_ID, WorkerInfo> & GroupDriver::allocate_workers(const uint16_t & num_requested_workers)
{
	driver.allocate_workers(ID, num_requested_workers);

	return workers;
}

vector<Worker_ID> GroupDriver::deallocate_workers(const vector<Worker_ID> & yielded_workers)
{
	return driver.deallocate_workers(ID, yielded_workers);
}

uint16_t GroupDriver::get_num_workers()
{
	return (uint16_t) workers.size();
}

uint64_t GroupDriver::get_num_rows(Matrix_ID & matrix_ID)
{
	return matrices[matrix_ID]->num_rows;
}

uint64_t GroupDriver::get_num_cols(Matrix_ID & matrix_ID)
{
	return matrices[matrix_ID]->num_cols;
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

string GroupDriver::list_all_workers()
{
	return driver.list_all_workers();
}

string GroupDriver::list_all_workers(const string & preamble)
{
	return driver.list_all_workers(preamble);
}

string GroupDriver::list_active_workers()
{
	return driver.list_active_workers();
}

string GroupDriver::list_active_workers(const string & preamble)
{
	return driver.list_active_workers(preamble);
}

string GroupDriver::list_inactive_workers()
{
	return driver.list_inactive_workers();
}

string GroupDriver::list_inactive_workers(const string & preamble)
{
	return driver.list_inactive_workers(preamble);
}

string GroupDriver::list_allocated_workers()
{
	return driver.list_allocated_workers(ID);
}

string GroupDriver::list_allocated_workers(const string & preamble)
{
	return driver.list_allocated_workers(ID, preamble);
}

Library_ID GroupDriver::load_library(string library_name, string library_path)
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

	    // copying the contents of the
	    // string to char array
	strcpy(library_name_c, library_name.c_str());
	strcpy(library_path_c, library_path.c_str());

//	auto library_name_c = library_name.c_str();
//	auto library_path_c = library_path.c_str();
	uint16_t library_name_c_length = strlen(library_name_c);
	uint16_t library_path_c_length = strlen(library_path_c);

	next_library_ID++;

	MPI_Bcast(&next_library_ID, 1, MPI_BYTE, 0, group);
	MPI_Bcast(&library_name_c_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(library_name_c, library_name_length+1, MPI_CHAR, 0, group);
	MPI_Bcast(&library_path_c_length, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(library_path_c, library_path_length+1, MPI_CHAR, 0, group);

	MPI_Barrier(group);

	char cstr[library_path.length()+1];
	std::strcpy(cstr, library_path.c_str());

	log->info("Loading library {} located at {}", library_name, library_path);

	void * lib = dlopen(library_path.c_str(), RTLD_LAZY);
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


//		Library * library = create_library(group);
	log->info("L 12");
	Library * library_ptr = reinterpret_cast<Library*>(create_library(group));

	log->info("L 13");
	libraries.insert(std::make_pair(next_library_ID, library_ptr));

//	string task_name = "greet";
//	Parameters in, out;

	log->info("L 14");
	library_ptr->load();
//	library_ptr->run(task_name, in, out);

	//    	libraries.insert(std::make_pair(library_name, LibraryInfo(library_name, library_path, lib, library)));

	//	if (!library->load()) {
	//		log->info("Library {} loaded", library_name);
	//
	//		Parameters input;
	//		Parameters output;
	//
	//		library->run("greet", input, output);
	//	}

	delete dlsym_error;

	log->info("L 16");
	MPI_Barrier(group);

	return next_library_ID;
}

void GroupDriver::add_worker(const Worker_ID & worker_ID, const WorkerInfo & info)
{
	workers.insert(std::make_pair(worker_ID, info));
}

void GroupDriver::remove_worker(const Worker_ID & worker_ID)
{
	workers.erase(workers.find(worker_ID));
	for (auto it = workers.begin(); it != workers.end(); it++)
					log->info("_____________ {}", it->first);
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

	Library_ID lib_ID = in_msg.read_library_ID();

	if (check_library_ID(lib_ID)) {
		log->info("L 17");
		string function_name = in_msg.read_string();
		log->info("L 18");

		deserialize_parameters(in, in_msg);
		log->info("L 19");

		libraries[lib_ID]->run(function_name, in, out);
		log->info("L 20");

		MPI_Barrier(group);

		int num_distmatrices;
		string distmatrix_name;
		uint16_t dmnl;
		uint64_t num_rows, num_cols;
		log->info("L 21");

		Worker_ID primary_worker = workers.begin()->first;

		MPI_Recv(&num_distmatrices, 1, MPI_INT, primary_worker, 0, group, &status);
		log->info("L 22 {}", num_distmatrices);

		if (num_distmatrices > 0) {
			Matrix_ID matrix_IDs[num_distmatrices];
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

				matrices.insert(std::make_pair(next_matrix_ID, std::make_shared<MatrixInfo>(next_matrix_ID, distmatrix_name, num_rows, num_cols, sparse, layout, num_partitions)));
				matrix_IDs[i] = next_matrix_ID++;
			}
			log->info("L 23");

			MPI_Bcast(&matrix_IDs, num_distmatrices, MPI_UNSIGNED_SHORT, 0, group);
			log->info("L 24");

			uint64_t worker_num_rows;
			uint64_t * row_indices;

			for (int i = 0; i < num_distmatrices; i++) {

				std::clock_t start;

				MPI_Bcast(&matrix_IDs[i], 1, MPI_UNSIGNED_SHORT, 0, group);
				MPI_Barrier(group);

				for (auto it = workers.begin(); it != workers.end(); it++) {

					Worker_ID id = it->first;
					MPI_Recv(&worker_num_rows, 1, MPI_UNSIGNED_LONG, id, 0, group, &status);

					row_indices = new uint64_t[worker_num_rows];

					MPI_Recv(row_indices, (int) worker_num_rows, MPI_UNSIGNED_LONG, id, 0, group, &status);
					for (uint64_t j = 0; j < worker_num_rows; j++)
						matrices[matrix_IDs[i]]->row_assignments[row_indices[j]] = id;

					delete [] row_indices;
				}
				out.add_matrix_info(matrices[matrix_IDs[i]]->name, matrices[matrix_IDs[i]]);
			}
		}
		log->info("L 25");

		MPI_Barrier(group);
		log->info("L 26");

		serialize_parameters(out, out_msg);
		log->info("L 27 {}", out.to_string());
	}

	log->info("L 28");
	out_msg.update_body_length();
	log->info("L 29");
	out_msg.update_datatype_count();
	log->info("L 30 {}", out_msg.to_string());

//	out_data = temp_out_msg.body();
//	log->info("L 31");
//	out_data_length = temp_out_msg.get_body_length();
//	log->info("L 32 {}", out_data_length);

}

void GroupDriver::run_task(const char * & in_data, uint32_t & in_data_length, char * & out_data, uint32_t & out_data_length, client_language cl)
{
	alchemist_command command = _AM_WORKER_RUN_TASK;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	MPI_Bcast(&in_data_length, 1, MPI_UNSIGNED, 0, group);
	MPI_Bcast((char *) in_data, in_data_length, MPI_CHAR, 0, group);

	MPI_Barrier(group);

	Message temp_in_msg(in_data_length), temp_out_msg;
	temp_out_msg.clear();
	temp_in_msg.set_client_language(cl);
	temp_out_msg.set_client_language(cl);
	Parameters in, out;

	temp_in_msg.copy_body(&in_data[0], in_data_length);
	MPI_Barrier(group);

	Library_ID lib_ID = temp_in_msg.read_uint8();

	if (check_library_ID(lib_ID)) {
		log->info("L 17");
		string function_name = temp_in_msg.read_string();
		log->info("L 18");

		deserialize_parameters(in, temp_in_msg);
		log->info("L 19");

		libraries[lib_ID]->run(function_name, in, out);
		log->info("L 20");

		MPI_Barrier(group);

		int num_distmatrices;
		string distmatrix_name;
		uint16_t dmnl;
		uint64_t num_rows, num_cols;
		log->info("L 21");

		Worker_ID primary_worker = workers.begin()->first;

		MPI_Recv(&num_distmatrices, 1, MPI_INT, primary_worker, 0, group, &status);
		log->info("L 22 {}", num_distmatrices);

		if (num_distmatrices > 0) {
			Matrix_ID matrix_IDs[num_distmatrices];
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

				matrices.insert(std::make_pair(next_matrix_ID, std::make_shared<MatrixInfo>(next_matrix_ID, distmatrix_name, num_rows, num_cols, sparse, layout, num_partitions)));
				matrix_IDs[i] = next_matrix_ID++;
			}
			log->info("L 23");

			MPI_Bcast(&matrix_IDs, num_distmatrices, MPI_UNSIGNED_SHORT, 0, group);
			log->info("L 24");

			uint64_t worker_num_rows;
			uint64_t * row_indices;

			for (int i = 0; i < num_distmatrices; i++) {

				std::clock_t start;

				MPI_Bcast(&matrix_IDs[i], 1, MPI_UNSIGNED_SHORT, 0, group);
				MPI_Barrier(group);

				for (auto it = workers.begin(); it != workers.end(); it++) {

					Worker_ID id = it->first;
					MPI_Recv(&worker_num_rows, 1, MPI_UNSIGNED_LONG, id, 0, group, &status);

					row_indices = new uint64_t[worker_num_rows];

					MPI_Recv(row_indices, (int) worker_num_rows, MPI_UNSIGNED_LONG, id, 0, group, &status);
					for (uint64_t j = 0; j < worker_num_rows; j++)
						matrices[matrix_IDs[i]]->row_assignments[row_indices[j]] = id;

					delete [] row_indices;
				}
				out.add_matrix_info(matrices[matrix_IDs[i]]->name, matrices[matrix_IDs[i]]);
			}
		}
		log->info("L 25");

		MPI_Barrier(group);
		log->info("L 26");

		serialize_parameters(out, temp_out_msg);
		log->info("L 27 {}", out.to_string());
	}

	log->info("L 28");
	temp_out_msg.update_body_length();
	log->info("L 29");
	temp_out_msg.update_datatype_count();
	log->info("L 30");

	out_data = temp_out_msg.body();
	log->info("L 31");
	out_data_length = temp_out_msg.get_body_length();
	log->info("L 32 {}", out_data_length);
}

void GroupDriver::deserialize_parameters(Parameters & p, Message & msg) {

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

			switch (dt) {
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
				p.add_matrix_info(name, matrices[msg.read_matrix_ID()]);
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
		case MATRIX_INFO:
			msg.add_matrix_info(*p.get_matrix_info(name));
			break;
		}

		dt = p.get_next_parameter();
	}
}

Matrix_ID GroupDriver::new_matrix(string name, uint64_t num_rows, uint64_t num_cols, uint8_t sparse, uint8_t layout)
{
	alchemist_command command = _AM_NEW_MATRIX;

	log->info("Sending command {} to workers", get_command_name(command));

	MPI_Request req;
	MPI_Status status;
	MPI_Ibcast(&command, 1, MPI_UNSIGNED_CHAR, 0, group, &req);
	MPI_Wait(&req, &status);

	Matrix_ID matrix_ID = next_matrix_ID++;

	MPI_Bcast(&matrix_ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Bcast(&num_rows, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&num_cols, 1, MPI_UNSIGNED_LONG, 0, group);
	MPI_Bcast(&sparse, 1, MPI_UNSIGNED_CHAR, 0, group);
	MPI_Bcast(&layout, 1, MPI_UNSIGNED_CHAR, 0, group);
	uint8_t num_paritions = (uint8_t) workers.size();

	MPI_Barrier(group);

	matrices.insert(std::make_pair(matrix_ID, std::make_shared<MatrixInfo>(matrix_ID, name, num_rows, num_cols, sparse > 0, layout, num_paritions)));

	MPI_Barrier(group);

	determine_row_assignments(matrix_ID);

	return matrix_ID;
}

Worker_ID * GroupDriver::get_row_assignments(Matrix_ID & matrix_ID)
{
	return matrices[matrix_ID]->row_assignments;
}

bool GroupDriver::check_library_ID(Library_ID & lib_ID)
{
	return true;
}

string GroupDriver::list_workers()
{
	return driver.list_all_workers();
}

MatrixInfo & GroupDriver::get_matrix_info(const Matrix_ID matrix_ID)
{
	return *matrices[matrix_ID];
}

void GroupDriver::determine_row_assignments(Matrix_ID & matrix_ID)
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

	MPI_Bcast(&matrices[matrix_ID]->ID, 1, MPI_UNSIGNED_SHORT, 0, group);
	MPI_Barrier(group);

	for (auto it = workers.begin(); it != workers.end(); it++) {
		Worker_ID id = it->first;
		MPI_Recv(&worker_num_rows, 1, MPI_UNSIGNED_LONG, id, 0, group, &status);

		row_indices = new uint64_t[worker_num_rows];

		MPI_Recv(row_indices, (int) worker_num_rows, MPI_UNSIGNED_LONG, id, 0, group, &status);
		for (uint64_t i = 0; i < worker_num_rows; i++) {
			matrices[matrix_ID]->row_assignments[row_indices[i]] = id;
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

//	for (int i = 0; i < num_client_workers; i++) {
//			for (int j = 0; j < num_alchemist_workers; j++)
//				std::cout << layout_rr[i][j][0] << "," << layout_rr[i][j][1] << " ";
//		std::cout << std::endl;
//	}

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

