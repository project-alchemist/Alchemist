#ifndef ALCHEMIST__GROUPWORKER_HPP
#define ALCHEMIST__GROUPWORKER_HPP

#include "Server.hpp"
#include "Worker.hpp"
#include "WorkerSession.hpp"
//#include "Library.hpp"

namespace alchemist {

class Worker;
class WorkerSession;

typedef std::shared_ptr<El::Grid> Grid_ptr;
typedef std::shared_ptr<Parameter> Parameter_ptr;
typedef std::shared_ptr<WorkerSession> WorkerSession_ptr;

class GroupWorker : public Server, public std::enable_shared_from_this<GroupWorker>
{
public:

//	GroupWorker(GroupID _groupID, WorkerID _workerID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//			const unsigned int port, Log_ptr & _log);
//	GroupWorker(GroupID _groupID, WorkerID _workerID, MPI_Comm & _group, MPI_Comm & _group_peers, io_context & _io_context,
//			const tcp::endpoint & endpoint, Log_ptr & _log);

	GroupWorker(GroupID _groupID, Worker & _worker, io_context & _io_context, const unsigned int port, bool _primary_group_worker, Log_ptr & _log);
	GroupWorker(GroupID _groupID, Worker & _worker, io_context & _io_context, const tcp::endpoint & endpoint, bool _primary_group_worker, Log_ptr & _log);
	~GroupWorker();

	WorkerID get_workerID();
	GroupID get_groupID();

	int start();

	Grid_ptr grid;

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

	void handle_get_process_grid();

	bool check_libraryID(LibraryID & libID);

	int get_matrix_rows();

	void print_data(MatrixID ID);

	void serialize_parameters(vector<Parameter_ptr> & out_parameters, Message & msg);
	void deserialize_parameters(vector<Parameter_ptr> & in_parameters, Message & msg);

	void set_group_comm(MPI_Comm & world, MPI_Group & temp_group);
	void set_group_peers_comm(MPI_Comm & world, MPI_Group & temp_group);

	uint64_t get_num_local_rows(MatrixID matrixID);
	uint64_t get_num_local_cols(MatrixID matrixID);

	void set_value(MatrixID matrixID, uint64_t row, uint64_t col, float value);
	void set_value(MatrixID matrixID, uint64_t row, uint64_t col, double value);

	void get_value(MatrixID matrixID, uint64_t row, uint64_t col, float & value);
	void get_value(MatrixID matrixID, uint64_t row, uint64_t col, double & value);

	int load_library();
	void run_task();

	WorkerID workerID;
private:
	MPI_Comm group;
	MPI_Comm group_peers;

	GroupID groupID;
	MatrixID current_matrixID;
	SessionID next_sessionID;

	Worker & worker;

	bool primary_group_worker;

	client_language cl;

	map<LibraryID, Library *> libraries;
	map<SessionID, WorkerSession_ptr> sessions;
	map<MatrixID, DistMatrix_ptr> matrices;

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

	void read_matrix_parameters(vector<Parameter_ptr> & out_parameters);

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
