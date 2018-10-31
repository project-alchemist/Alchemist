#ifndef ALCHEMIST__GROUPDRIVER_HPP
#define ALCHEMIST__GROUPDRIVER_HPP

#include "Alchemist.hpp"
#include "Driver.hpp"
#include "DriverSession.hpp"

namespace alchemist {

class Driver;
class DriverSession;

using Eigen::MatrixXd;
using Eigen::VectorXd;

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

class GroupDriver : public std::enable_shared_from_this<GroupDriver>
{
public:
	GroupDriver(Group_ID _ID, Driver & _driver);
	GroupDriver(Group_ID _ID, Driver & _driver, Log_ptr & _log);
	~GroupDriver();

	void start(tcp::socket socket);

	void say_something();

	client_language get_client_language() { return cl; }

	DriverSession_ptr session;

	vector<Worker_ID> worder_IDs;

	void set_group_comm(MPI_Comm & world, MPI_Group & temp_group);

	map<Worker_ID, WorkerInfo> allocate_workers(const uint16_t & num_requested_workers);
	void deallocate_workers();

	string list_workers();
	uint16_t get_num_workers();

	string list_sessions();
	int load_library(string library_name, string library_path);
	Matrix_ID new_matrix(unsigned char type, unsigned char layout, uint64_t num_rows, uint64_t num_cols);
	vector<uint16_t> & get_row_assignments(Matrix_ID & matrix_ID);
	void determine_row_assignments(Matrix_ID & matrix_ID);
	vector<vector<vector<float> > > prepare_data_layout_table(uint16_t num_alchemist_workers, uint16_t num_client_workers);
	int read_HDF5();

	int run_task(string task, Matrix_ID matrix_ID, uint32_t rank, uint8_t method);
//	int run_task(const char * data, uint32_t data_length);
	int process_input_parameters(Parameters & input_parameters);
	int process_output_parameters(Parameters & output_parameters);

	int truncated_SVD(Matrix_ID matrix_ID, uint32_t rank, uint8_t method);

	uint64_t get_num_rows(Matrix_ID & matrix_ID);
	uint64_t get_num_cols(Matrix_ID & matrix_ID);
private:
	MPI_Comm group;

	Group_ID ID;
	client_language cl;

	Library * library;

	map<Worker_ID, WorkerInfo> workers;
	map<Matrix_ID, MatrixInfo> matrices;

	Driver & driver;

	Matrix_ID next_matrix_ID;

	Log_ptr log;
};

typedef std::shared_ptr<GroupDriver> GroupDriver_ptr;

}

#endif
