#ifndef ALCHEMIST__DRIVER_HPP
#define ALCHEMIST__DRIVER_HPP

#include "Worker.hpp"
#include "DriverSession.hpp"

namespace alchemist {

class DriverSession;

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

using std::string;

class Driver : public Server, public std::enable_shared_from_this<Driver>
{
public:

	Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const unsigned int port);
	Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint);
	~Driver();

	int start();

	string list_workers();
	string list_inactive_workers();
	string list_active_workers();
	string list_allocated_workers(DriverSession_ptr);
	string list_sessions();

	void add_session(DriverSession_ptr session);
	void remove_session(DriverSession_ptr session);

	void idle_workers();
	void print_workers();

	void go();

	std::map<Worker_ID, WorkerInfo> allocate_workers(DriverSession_ptr, uint16_t);
	void deallocate_workers(DriverSession_ptr);

private:
	MPI_Comm & world;
	MPI_Comm & peers;

	uint16_t num_workers;

	void print_num_sessions();
//	void handshake(const Session_ptr session, Message & msg);
	int get_num_sessions();

	std::map<Session_ID, DriverSession_ptr> sessions;

//	std::map<MatrixHandle, MatrixDescriptor> matrices;
	std::map<Worker_ID, WorkerInfo> workers;

	std::map<Worker_ID, Session_ID> active_workers;
	std::vector<Worker_ID> inactive_workers;

	std::map<Session_ID, std::map<Worker_ID, WorkerInfo> > allocated_workers;

	uint32_t next_matrix_ID;

	// ====================================   UTILITY FUNCTIONS   ====================================

	// ----------------------------------------   Printing   -----------------------------------------

	void print_welcome_message();
	void print_ready_message();

	string make_daytime_string();

	// -----------------------------------------   Workers   -----------------------------------------

	int start_workers();
	int register_workers();
	void open_workers();

	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

	// ====================================   COMMAND FUNCTIONS   ====================================

	int handle_command(int command_code);
	int run_task();

	int shut_down();

	// ---------------------------------------   Information   ---------------------------------------

	int send_assigned_workers_info();

	// ----------------------------------------   Parameters   ---------------------------------------

	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	// -----------------------------------------   Library   -----------------------------------------

	int load_library() ;

	// -----------------------------------------   Testing   -----------------------------------------

	int receive_test_string(const DriverSession_ptr, const char *, const uint32_t);
	int send_test_string(DriverSession_ptr);

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
