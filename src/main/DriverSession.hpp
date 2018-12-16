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

	void remove_session();

	void request_matrix();

	void send_matrix_info(Matrix_ID matrix_ID);
	void send_matrix_ID(Matrix_ID & matrix_ID);

	bool send_response_string();


	int handle_message();

	std::shared_ptr<DriverSession> shared_from_this()
	{
		return std::static_pointer_cast<DriverSession>(Session::shared_from_this());
	}

private:
	GroupDriver & group_driver;

	uint16_t num_group_workers;
	string log_dir;

//	void handle_handshake();
	void handle_request_ID();
	void handle_client_info();
	void handle_send_test_string();
	void handle_request_test_string();
	void handle_close_connection();
	void handle_request_workers();
	void handle_yield_workers();
	void handle_send_assigned_worker_info();
	void handle_list_all_workers();
	void handle_list_active_workers();
	void handle_list_inactive_workers();
	void handle_list_assigned_workers();
	void handle_list_available_libraries();
	void handle_load_library();
	void handle_unload_library();
	void handle_matrix_info();
	void handle_matrix_layout();
	void handle_send_matrix_blocks();
	void handle_request_matrix_blocks();
	void handle_run_task();
	void handle_invalid_command();
	void handle_shutdown();

	void send_layout(vector<vector<uint32_t> > & rows_on_workers);
	void send_layout(vector<uint16_t> & row_assignments);

};

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__DRIVERSESSION_HPP
