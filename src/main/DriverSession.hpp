#ifndef ALCHEMIST__DRIVERSESSION_HPP
#define ALCHEMIST__DRIVERSESSION_HPP


#include "Session.hpp"
#include "GroupDriver.hpp"


namespace alchemist {

class GroupDriver;

class DriverSession : public Session
{
public:
	DriverSession(tcp::socket, GroupDriver &);
	DriverSession(tcp::socket, GroupDriver &, Client_ID _client_ID);
	DriverSession(tcp::socket, GroupDriver &, Client_ID _client_ID, Log_ptr &);

	map<Worker_ID, WorkerInfo> workers;

	vector<Worker_ID>  allocated_workers;
	vector<Library_ID> loaded_libraries;

	void start();

	bool send_response_string();
	bool send_test_string();

	void remove_session();

	bool receive_client_info();
	void request_matrix();

	bool allocate_workers();
	bool deallocate_workers();

	bool list_all_workers();
	bool list_active_workers();
	bool list_inactive_workers();
	bool list_assigned_workers();

	bool load_library();
	bool run_task();
	bool unload_library();

	void new_matrix();

	void send_matrix_info(Matrix_ID matrix_ID);
	void send_matrix_ID(Matrix_ID & matrix_ID);
	void send_layout(vector<vector<uint32_t> > & rows_on_workers);
	void send_layout(vector<uint16_t> & row_assignments);

	int handle_message();

	std::shared_ptr<DriverSession> shared_from_this()
	{
		return std::static_pointer_cast<DriverSession>(Session::shared_from_this());
	}

private:
	GroupDriver & group_driver;

	uint16_t num_group_workers;
	string log_dir;
};

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__DRIVERSESSION_HPP
