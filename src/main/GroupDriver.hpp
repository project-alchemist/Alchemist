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
	GroupDriver(GroupID ID, Driver & _driver);
	GroupDriver(GroupID ID, Driver & _driver, Log_ptr & _log);
	~GroupDriver();

	void start(tcp::socket socket);

	client_language get_client_language() { return cl; }

	DriverSession_ptr session;

	map<WorkerID, WorkerInfo_ptr> workers;

	void free_group();
	void ready_group();
	void set_group_comm(MPI_Comm & world, MPI_Group & temp_group);

	const map<WorkerID, WorkerInfo_ptr> & allocate_workers(const uint16_t & num_requested_workers);
	vector<WorkerID> deallocate_workers(const vector<WorkerID> & yielded_workers);

	string list_workers();
	uint16_t get_num_workers();

	MatrixInfo_ptr get_matrix_info(const MatrixID matrixID);

	void get_process_grid();

	string list_sessions();
	LibraryID load_library(string library_name, string library_path);
	MatrixID new_matrix(const MatrixInfo_ptr x);
	MatrixInfo_ptr new_matrix(const string name, const uint64_t num_rows, const uint64_t num_cols, const uint8_t sparse, const layout l);
	void determine_worker_assignments(MatrixID & matrixID);
	vector<vector<vector<float> > > prepare_data_layout_table(uint16_t num_alchemist_workers, uint16_t num_client_workers);


	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

//	int run_task(LibraryID libID, string task, ArrayID matrixID, uint32_t rank, uint8_t method);
	void run_task(const char * & in_data, uint32_t & in_data_length, char * & out_data, uint32_t & out_data_length, client_language cl);
	void run_task(Message & in, Message & out);
	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	void serialize_parameters(Parameters & output_parameters, Message & msg);
	void deserialize_parameters(Parameters & input_parameters, Message & msg);

	bool check_libraryID(LibraryID & libID);

	uint64_t get_num_rows(MatrixID & matrixID);
	uint64_t get_num_cols(MatrixID & matrixID);

	vector<WorkerInfo_ptr> get_all_workers();
	vector<WorkerInfo_ptr> get_active_workers();
	vector<WorkerInfo_ptr> get_inactive_workers();
	vector<WorkerInfo_ptr> get_assigned_workers();

	void open_workers();
	void close_workers();

	void print_info();

	void add_worker(const WorkerID & workerID, const WorkerInfo_ptr & info);
	void remove_worker(const WorkerID & workerID);

	void idle_workers();
private:
	MPI_Comm group;

	GroupID ID;
	client_language cl;

	map<LibraryID, Library *> libraries;
	map<ArrayID, MatrixInfo_ptr> matrices;

	Driver & driver;

	MatrixID next_matrixID;
	LibraryID next_libraryID;

	Log_ptr log;
};

typedef std::shared_ptr<GroupDriver> GroupDriver_ptr;

}

#endif
