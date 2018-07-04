#ifndef ALCHEMIST__WORKER_HPP
#define ALCHEMIST__WORKER_HPP

#include "Server.hpp"
#include "WorkerSession.hpp"

namespace alchemist {

class WorkerSession;

typedef std::shared_ptr<WorkerSession> WorkerSession_ptr;

using std::string;

class Worker : public Server, public std::enable_shared_from_this<Worker>
{
public:

	Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port);
	Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint);
	~Worker();

	int start();

	Worker_ID get_ID();

	void add_session(WorkerSession_ptr session);
	void remove_session(WorkerSession_ptr session);

	int handle_message(WorkerSession_ptr session, Message & msg);

private:
	MPI_Comm & world;
	MPI_Comm & peers;

	Worker_ID ID;

	std::map<Session_ID, WorkerSession_ptr> sessions;
	WorkerSession_ptr ses;

	bool accept_connections;

	void print_num_sessions();
//	void handshake(const Session_ptr session, Message & msg);
	int get_num_sessions();


	std::vector<boost::thread> some_threads;

	// ====================================   UTILITY FUNCTIONS   ====================================

	string list_workers();
	string list_inactive_workers();
	string list_active_workers();
	string list_sessions();

	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

	// ====================================   COMMAND FUNCTIONS   ====================================

	int wait_for_command();

	int send_info();
	int run_task();

	void handle_command(alchemist_command c);

	// ---------------------------------------   Information   ---------------------------------------

	// ----------------------------------------   Parameters   ---------------------------------------

	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	// -----------------------------------------   Library   -----------------------------------------

	int load_library() ;

	// -----------------------------------------   Testing   -----------------------------------------

	int receive_test_string(const WorkerSession_ptr, const char *, const uint32_t);
	int send_test_string(WorkerSession_ptr);

	// -----------------------------------------   Matrices   ----------------------------------------

//	MatrixHandle register_matrix(size_t num_rows, size_t num_cols);
	int receive_new_matrix();
	int get_matrix_dimensions();
	int get_transpose();
	int matrix_multiply();
	int get_matrix_rows();

	void accept_connection();
};

}

#endif
