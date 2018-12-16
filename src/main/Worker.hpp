#ifndef ALCHEMIST__WORKER_HPP
#define ALCHEMIST__WORKER_HPP

#include "GroupWorker.hpp"

namespace alchemist {

class GroupWorker;

typedef std::shared_ptr<GroupWorker> GroupWorker_ptr;

class Worker : public std::enable_shared_from_this<Worker>
{
public:

	Worker(io_context & _io_context, const unsigned int port);
//	Worker(io_context & _io_context, const tcp::endpoint & endpoint);
	~Worker();

	void display_info();
	void print_info();

	Worker_ID get_ID();

//	Worker_ID get_ID();
//	bool is_active();
//
//	int new_matrix();
//	int get_matrix_layout();
//
//	int receive_new_matrix();
//	int get_matrix_dimensions();
//	int get_transpose();
//	int matrix_multiply();
//
//	int get_matrix_rows();
//
//	void print_data(Matrix_ID ID);
//
//
//	void set_value(Matrix_ID ID, uint64_t row, uint64_t col, float value);
//	void set_value(Matrix_ID ID, uint64_t row, uint64_t col, double value);
//
//	void get_value(Matrix_ID ID, uint64_t row, uint64_t col, float & value);
//	void get_value(Matrix_ID ID, uint64_t row, uint64_t col, double & value);


private:
	MPI_Comm world;
	MPI_Comm group;
	MPI_Comm group_peers;


	string hostname;
	string address;
	uint16_t port;

	GroupWorker_ptr group_worker;

	Worker_ID ID;
	Client_ID client_ID;
	Session_ID next_session_ID;

	bool accept_connections;

	io_context & ic;
	tcp::endpoint endpoint;
	Log_ptr log;

	vector<std::thread> threads;

	// ================================================================================================

	// ------------------------------------   Command Management   ------------------------------------

	int wait_for_command();
	int handle_command(alchemist_command c);

	// --------------------------------------   Initialization   --------------------------------------

	int start();
	void send_info();
	void handle_new_group();
	void get_group_peers();

//	string session_preamble();
//
//	int receive_test_string(const char * data, const uint32_t length);
//	int send_test_string();
//	string get_test_string();
//
//	// ------------------------------------   Utility functions   ------------------------------------
//
//
//	// ---------------------------------------   Parameters   ----------------------------------------
//
//	int process_input_parameters(Parameters & input_parameters);
//	int process_output_parameters(Parameters & output_parameters);
//
//	// -------------------------------------   Client Management   -----------------------------------
//
//	int new_client();
//	int close_client();
//
//	// ------------------------------------   Session Management   -----------------------------------
//
//	int add_session();
//	int end_session();
//	int remove_session();
//
//	string list_sessions();
//	int print_num_sessions();
//
//	// ------------------------------------   Library Management   -----------------------------------
//
//	int load_library() ;
//	int run_task();
//
//
//
//	// -----------------------------------------   File I/O   ----------------------------------------
//
//	int read_HDF5();
};

}

#endif
