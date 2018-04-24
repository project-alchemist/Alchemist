#ifndef ALCHEMIST__WORKER_HPP
#define ALCHEMIST__WORKER_HPP

#include "Executor.hpp"
#include "Server.hpp"
#include "Library.hpp"
#include "utility/logging.hpp"

namespace alchemist {

using std::string;

typedef uint16_t Worker_ID;

struct WorkerInfo {
	WorkerInfo(): hostname(string("0")), port(0) {}
	WorkerInfo(string _hostname, uint16_t _port): hostname(_hostname), port(_port) {}

	string hostname;
	uint16_t port;
};

class Worker : public Executor, public Server, public std::enable_shared_from_this<Worker>
{
public:

	Worker(MPI_Comm & _world, MPI_Comm & _peers, io_context & _io_context, const tcp::endpoint & endpoint);
	~Worker();

	int start();

private:
	Worker_ID ID;
	Log_ptr log;

	// ====================================   UTILITY FUNCTIONS   ====================================

	// ----------------------------------------   File I/O   ----------------------------------------

	int read_HDF5();

	// ====================================   COMMAND FUNCTIONS   ====================================


	int handle_message(Session_ptr session, const Message & msg);
	int wait_for_command();

	int send_info();
	int run_task();

	// ---------------------------------------   Information   ---------------------------------------

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
