#ifndef ALCHEMIST__GROUPDRIVER_HPP
#define ALCHEMIST__GROUPDRIVER_HPP

#include "Alchemist.hpp"
#include "Driver.hpp"
#include "DriverSession.hpp"
#include "Library.hpp"

namespace alchemist {

class Driver;
class DriverSession;

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

class GroupDriver : public std::enable_shared_from_this<GroupDriver>
{
public:
	GroupDriver(Group_ID _ID, Driver & _driver);
	GroupDriver(Group_ID _ID, Driver & _driver, Log_ptr & _log);
	~GroupDriver();

	void start(tcp::socket socket);

	client_language get_client_language() { return cl; }

	DriverSession_ptr session;

	map<Worker_ID, WorkerInfo> workers;

	void free_group();
	void ready_group();
	void set_group_comm(MPI_Comm & world, MPI_Group & temp_group);

	const map<Worker_ID, WorkerInfo> & allocate_workers(const uint16_t & num_requested_workers);
	vector<Worker_ID> deallocate_workers(const vector<Worker_ID> & yielded_workers);

	string list_workers();
	uint16_t get_num_workers();

	MatrixInfo & get_matrix_info(const Matrix_ID matrix_ID);

	string list_sessions();
	Library_ID load_library(string library_name, string library_path);
	Matrix_ID new_matrix(string name, uint64_t num_rows, uint64_t num_cols, uint8_t sparse, uint8_t layout);
	Worker_ID * get_row_assignments(Matrix_ID & matrix_ID);
	void determine_row_assignments(Matrix_ID & matrix_ID);
	vector<vector<vector<float> > > prepare_data_layout_table(uint16_t num_alchemist_workers, uint16_t num_client_workers);


	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

//	int run_task(Library_ID lib_ID, string task, Matrix_ID matrix_ID, uint32_t rank, uint8_t method);
	void run_task(const char * & in_data, uint32_t & in_data_length, char * & out_data, uint32_t & out_data_length, client_language cl);
	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	void serialize_parameters(Parameters & output_parameters, Message & msg);
	void deserialize_parameters(Parameters & input_parameters, Message & msg);

	bool check_library_ID(Library_ID & lib_ID);

	uint64_t get_num_rows(Matrix_ID & matrix_ID);
	uint64_t get_num_cols(Matrix_ID & matrix_ID);

	string list_all_workers();
	string list_all_workers(const string & preamble);
	string list_active_workers();
	string list_active_workers(const string & preamble);
	string list_inactive_workers();
	string list_inactive_workers(const string & preamble);
	string list_allocated_workers();
	string list_allocated_workers(const string & preamble);

	void open_workers();
	void close_workers();

	void print_info();

	void add_worker(const Worker_ID & worker_ID, const WorkerInfo & info);
	void remove_worker(const Worker_ID & worker_ID);

	void idle_workers();
private:
	MPI_Comm group;

	Group_ID ID;
	client_language cl;

	map<Library_ID, Library *> libraries;
	map<Matrix_ID, MatrixInfo_ptr> matrices;

	Driver & driver;

	Matrix_ID next_matrix_ID;
	Library_ID next_library_ID;

	Log_ptr log;
};

typedef std::shared_ptr<GroupDriver> GroupDriver_ptr;

}

#endif
