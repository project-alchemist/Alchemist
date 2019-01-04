#ifndef ALCHEMIST__GROUPWORKER_HPP
#define ALCHEMIST__GROUPWORKER_HPP

#include "Server.hpp"
#include "Worker.hpp"
#include "WorkerSession.hpp"
#include "Library.hpp"

namespace alchemist {
typedef El::AbstractDistMatrix<double> ElDistMatrix;

class Worker;
class WorkerSession;

typedef std::shared_ptr<WorkerSession> WorkerSession_ptr;

class GroupWorker : public Server, public std::enable_shared_from_this<GroupWorker>
{
public:

//	GroupWorker(Group_ID _group_ID, Worker_ID _worker_ID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//			const unsigned int port, Log_ptr & _log);
//	GroupWorker(Group_ID _group_ID, Worker_ID _worker_ID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//			const tcp::endpoint & endpoint, Log_ptr & _log);

	GroupWorker(Group_ID _group_ID, Worker & _worker, io_context & _io_context, const unsigned int port, Log_ptr & _log);
	GroupWorker(Group_ID _group_ID, Worker & _worker, io_context & _io_context, const tcp::endpoint & endpoint, Log_ptr & _log);
	~GroupWorker();

	Worker_ID get_worker_ID();
	Group_ID get_group_ID();

	int start();

	uint32_t current_grid;
	vector<Grid_ptr> grids;

	// -------------------------------------   Matrix Management   -----------------------------------

	//	MatrixHandle register_matrix(size_t num_rows, size_t num_cols);

	void handle_print_info();
	void handle_group_open_connections();
	void handle_group_close_connections();
	void handle_free_group();

	int new_matrix();
	int get_matrix_layout();

	int receive_new_matrix();
	int get_matrix_dimensions();
	int get_transpose();
	int matrix_multiply();

	int get_matrix_rows();

	void print_data(Matrix_ID ID);

	void serialize_parameters(Parameters & output_parameters, Message & msg);
	void deserialize_parameters(Parameters & input_parameters, Message & msg);

	void set_group_comm(MPI_Comm & world, MPI_Group & temp_group);
	void set_group_peers_comm(MPI_Comm & world, MPI_Group & temp_group);

	void set_value(Matrix_ID ID, uint64_t row, uint64_t col, float value);
	void set_value(Matrix_ID ID, uint64_t row, uint64_t col, double value);

	void get_value(Matrix_ID ID, uint64_t row, uint64_t col, float & value);
	void get_value(Matrix_ID ID, uint64_t row, uint64_t col, double & value);

	int load_library();
	void run_task();

private:
	MPI_Comm group;
	MPI_Comm group_peers;

	Worker_ID worker_ID;
	Group_ID group_ID;
	Session_ID next_session_ID;

	Worker & worker;

	client_language cl;

	map<Library_ID, Library *> libraries;
	map<Session_ID, WorkerSession_ptr> sessions;
	map<Matrix_ID, DistMatrix_ptr> matrices;

	bool connection_open;

	vector<std::thread> threads;

//	int load_library();


	// ================================================================================================

	// ------------------------------------   Command Management   ------------------------------------

	int wait_for_command();
	int handle_command(alchemist_command c);

	// --------------------------------------   Initialization   --------------------------------------

	int accept_connection();

//	string preamble();
	string client_preamble();
//	string session_preamble();

	int receive_test_string(const char * data, const uint32_t length);
	int send_test_string();
	string get_test_string();

	// ------------------------------------   Utility functions   ------------------------------------


	// ---------------------------------------   Parameters   ----------------------------------------

	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	// -------------------------------------   Client Management   -----------------------------------

	int new_client();
	int close_client();

	// ------------------------------------   Session Management   -----------------------------------

	int add_session();
	int new_session(tcp::socket socket);
	int start_new_session();
	int end_session();
	int remove_session();

	string list_sessions();
	int print_num_sessions();

	// ------------------------------------   Library Management   -----------------------------------

//	int load_library();
//	int run_task();



	// -----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();
};

}

#endif
