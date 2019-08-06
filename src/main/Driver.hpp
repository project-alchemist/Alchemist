#ifndef ALCHEMIST__DRIVER_HPP
#define ALCHEMIST__DRIVER_HPP

#include "Worker.hpp"
#include "GroupDriver.hpp"
//#include "DriverSession.hpp"

namespace alchemist {

class GroupDriver;

typedef std::shared_ptr<GroupDriver> GroupDriver_ptr;

class Driver : public Server, public std::enable_shared_from_this<Driver>
{
public:

	Driver(io_context & _io_context, const unsigned int port);
	Driver(io_context & _io_context, const tcp::endpoint & endpoint);
	~Driver();

	int start();

	// -----------------------------------------   Workers   -----------------------------------------

	vector<WorkerID> allocate_workers(const GroupID groupID, const uint16_t & num_requested_workers);
	vector<WorkerID> deallocate_workers(const GroupID groupID, const vector<WorkerID> & selected_workers);

	vector<WorkerInfo_ptr> get_all_workers();
	vector<WorkerInfo_ptr> get_active_workers();
	vector<WorkerInfo_ptr> get_inactive_workers();
	vector<WorkerInfo_ptr> get_assigned_workers(const GroupID groupID);

	string list_all_workers();
	string list_all_workers(const string & preamble);
	string list_active_workers();
	string list_active_workers(const string & preamble);
	string list_inactive_workers();
	string list_inactive_workers(const string & preamble);
	string list_allocated_workers(const GroupID groupID);
	string list_allocated_workers(const GroupID groupID, const string & preamble);

	void print_workers();
	void idle_workers();


	string list_sessions();

//	void add_session(DriverSession_ptr session);
//	void remove_session(DriverSession_ptr session);


	vector<uint16_t> & get_row_assignments(ArrayID & matrixID);
	void determine_row_assignments(ArrayID & matrixID);

	uint16_t get_num_workers();

	int load_library(string library_name, string library_path);

//	vector<vector<uint32_t> > new_matrix(unsigned char type, unsigned char layout, uint32_t num_rows, uint32_t num_cols);
//	MatrixID new_matrix(unsigned char type, unsigned char layout, uint64_t num_rows, uint64_t num_cols);

	vector<vector<vector<float> > > prepare_data_layout_table(uint16_t num_alchemist_workers, uint16_t num_client_workers);

private:
	MPI_Comm world;

	std::mutex worker_mutex;			// For safe access during worker allocation

	GroupID next_groupID;

	void print_num_sessions();
//	void handshake(const Session_ptr session, Message & msg);
	int get_num_sessions();

	map<GroupID, GroupDriver_ptr> groups;

	void print_group(GroupID groupID);

	ArrayID next_matrixID;

	// ====================================   UTILITY FUNCTIONS   ====================================

	// ----------------------------------------   Printing   -----------------------------------------

	void print_welcome_message();
	void print_ready_message();


	// ====================================   UTILITY FUNCTIONS   ====================================

	// -----------------------------------------   Groups   ------------------------------------------

	void new_group(tcp::socket socket);
	void reset_group(const GroupID & groupID);

	void set_group_communicator(const GroupID & groupID);

	// -----------------------------------------   Workers   -----------------------------------------

	uint16_t num_workers;

	map<WorkerID, WorkerInfo_ptr> workers;

	map<GroupID, map<WorkerID, WorkerInfo_ptr> > allocated_workers;
	vector<WorkerID> unallocated_workers;

	int start_workers();
	int register_workers();


	// ====================================   COMMAND FUNCTIONS   ====================================

	int handle_command(int command_code);
	int run_task();

	int shut_down();

	// ---------------------------------------   Information   ---------------------------------------

	int send_assigned_workers_info();

	// ----------------------------------------   Parameters   ---------------------------------------
//
//	int process_input_parameters(Parameters & input_parameters);
//	int process_output_parameters(Parameters & output_parameters);

	// -----------------------------------------   Library   -----------------------------------------


	// -----------------------------------------   Testing   -----------------------------------------

//	int receive_test_string(const DriverSession_ptr, const char *, const uint32_t);
//	int send_test_string(DriverSession_ptr);

	// -----------------------------------------   Matrices   ----------------------------------------

////	MatrixHandle register_matrix(size_t num_rows, size_t num_cols);
//	int receive_new_matrix();
//	int get_matrix_dimensions();
//	int get_transpose();
//	int matrix_multiply();
//	int get_matrix_rows();

	int start_new_session();
	int accept_connection();
};

}

#endif
