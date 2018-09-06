#ifndef ALCHEMIST__WORKERSESSION_HPP
#define ALCHEMIST__WORKERSESSION_HPP


#include "Alchemist.hpp"
#include "Message.hpp"
//#include "data_stream.hpp"
#include "Session.hpp"
#include "Worker.hpp"
#include "utility/logging.hpp"


namespace alchemist {

typedef uint16_t Session_ID;
typedef std::deque<Message> Message_queue;

class Worker;

class WorkerSession : public Session
{
public:
	WorkerSession(tcp::socket, Worker &);
	WorkerSession(tcp::socket, Worker &, uint16_t);
	WorkerSession(tcp::socket, Worker &, uint16_t, Log_ptr &);

	int handle_message();

	void start();

	bool send_response_string();
	bool send_test_string();

	bool receive_data();

	void remove_session();

	std::shared_ptr<WorkerSession> shared_from_this()
	{
		return std::static_pointer_cast<WorkerSession>(Session::shared_from_this());
	}

private:
	Worker & worker;
};

typedef std::shared_ptr<WorkerSession> WorkerSession_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__WORKERSESSION_HPP
