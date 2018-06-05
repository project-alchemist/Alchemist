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

	void read_header();
	void read_body();

	bool allocate_workers();
	bool deallocate_workers();

	bool list_all_workers();
	bool list_active_workers();
	bool list_inactive_workers();
	bool list_assigned_workers();

	int handle_message();

	std::shared_ptr<DriverSession> shared_from_this()
	{
		return std::static_pointer_cast<DriverSession>(Session::shared_from_this());
	}
private:
	Driver & driver;
};

typedef std::shared_ptr<DriverSession> DriverSession_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__DRIVERSESSION_HPP
