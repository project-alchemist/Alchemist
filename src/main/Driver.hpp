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

	string list_workers();
	string list_inactive_workers();
	string list_active_workers();
	string list_allocated_workers(Group_ID group_ID);
	string list_sessions();

//	void add_session(DriverSession_ptr session);
//	void remove_session(DriverSession_ptr session);

	void idle_workers();
	void print_workers();

	vector<uint16_t> & get_row_assignments(Matrix_ID & matrix_ID);
	void determine_row_assignments(Matrix_ID & matrix_ID);


	uint16_t get_num_workers();


	map<Worker_ID, WorkerInfo> allocate_workers(const Group_ID & group_ID, const uint16_t & num_requested_workers);
	void deallocate_workers(Group_ID group_ID);

	int load_library(string library_name, string library_path);

//	vector<vector<uint32_t> > new_matrix(unsigned char type, unsigned char layout, uint32_t num_rows, uint32_t num_cols);
//	Matrix_ID new_matrix(unsigned char type, unsigned char layout, uint64_t num_rows, uint64_t num_cols);

	vector<vector<vector<float> > > prepare_data_layout_table(uint16_t num_alchemist_workers, uint16_t num_client_workers);

private:
	MPI_Comm world;

	Group_ID next_group_ID;

	uint16_t num_workers;
	LibraryManager * lm;

	void print_num_sessions();
//	void handshake(const Session_ptr session, Message & msg);
	int get_num_sessions();

	map<Group_ID, GroupDriver_ptr> groups;
	map<Worker_ID, WorkerInfo> workers;


	map<Worker_ID, Group_ID> allocated_workers;
	vector<Worker_ID> unallocated_workers;

	Matrix_ID next_matrix_ID;

	// ====================================   UTILITY FUNCTIONS   ====================================

	// ----------------------------------------   Printing   -----------------------------------------

	void print_welcome_message();
	void print_ready_message();


	// ====================================   UTILITY FUNCTIONS   ====================================

	void new_group(tcp::socket socket);

	// -----------------------------------------   Workers   -----------------------------------------

	int start_workers();
	int register_workers();

	void open_workers(const Group_ID & group_ID, map<Worker_ID, WorkerInfo> & allocated_group);

	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

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
