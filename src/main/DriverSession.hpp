#ifndef ALCHEMIST__DRIVERSESSION_HPP
#define ALCHEMIST__DRIVERSESSION_HPP


#include "Alchemist.hpp"
#include "Message.hpp"
//#include "data_stream.hpp"
#include "Session.hpp"
#include "Driver.hpp"


namespace alchemist {

using boost::asio::ip::tcp;
using std::string;
using std::vector;

typedef uint16_t Session_ID;
typedef std::deque<Message> Message_queue;

class Driver;

class DriverSession : public Session
{
public:
	DriverSession(tcp::socket, Driver &);
	DriverSession(tcp::socket, Driver &, uint16_t);
	DriverSession(tcp::socket, Driver &, uint16_t, Log_ptr &);

	std::map<Worker_ID, WorkerInfo> workers;

	void start();

	bool send_response_string();
	bool send_test_string();

	void remove_session();

	bool receive_client_info();

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
	Driver & driver;

	uint16_t num_client_workers;
	string log_dir;
};

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__DRIVERSESSION_HPP
