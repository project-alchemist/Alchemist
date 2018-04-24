#ifndef ALCHEMIST__DRIVER_HPP
#define ALCHEMIST__DRIVER_HPP

#include "Executor.hpp"
#include "Server.hpp"
#include "Worker.hpp"
#include "Library.hpp"
#include "utility/logging.hpp"

namespace alchemist {

using std::string;

class Driver : public Executor, public Server, public std::enable_shared_from_this<Driver>
{
public:

	Driver(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint);
	~Driver();

	int start();

private:
	uint16_t num_workers;

	Log_ptr log;

//	std::map<MatrixHandle, MatrixDescriptor> matrices;
	std::map<Worker_ID, WorkerInfo> workers;

	std::map<Worker_ID, Session_ID> active_workers;
	std::vector<Worker_ID> inactive_workers;

	std::map<Session_ID, std::vector<Worker_ID> > assigned_workers;

	uint32_t next_matrix_ID;

	// ====================================   UTILITY FUNCTIONS   ====================================

	// ----------------------------------------   Printing   -----------------------------------------

	void print_welcome_message();
	void print_ready_message();

	void print_inactive_workers();
	void print_active_workers();
	void print_sessions();

	string make_daytime_string();

	// -----------------------------------------   Workers   -----------------------------------------

	int start_workers();
	int register_workers();
	int assign_workers();

	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

	// ====================================   COMMAND FUNCTIONS   ====================================

	int handle_message(Session_ptr session, const Message & msg);
	int handle_command(int command_code);
	int run_task();

	int shut_down();

	// ---------------------------------------   Information   ---------------------------------------

	int send_assigned_worker_info();

	// ----------------------------------------   Parameters   ---------------------------------------

	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	// -----------------------------------------   Library   -----------------------------------------

	int load_library() ;

	// -----------------------------------------   Testing   -----------------------------------------

	int receive_test_string(const Session_ptr, const char *, const uint32_t);
	int send_test_string(Session_ptr);

	// -----------------------------------------   Matrices   ----------------------------------------

//	MatrixHandle register_matrix(size_t num_rows, size_t num_cols);
	int receive_new_matrix();
	int get_matrix_dimensions();
	int get_transpose();
	int matrix_multiply();
	int get_matrix_rows();
};

}

#endif
