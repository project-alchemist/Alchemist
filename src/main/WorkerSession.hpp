#ifndef ALCHEMIST__WORKERSESSION_HPP
#define ALCHEMIST__WORKERSESSION_HPP


#include "Session.hpp"
#include "GroupWorker.hpp"


namespace alchemist {

class GroupWorker;

class WorkerSession : public Session
{
public:
	WorkerSession(tcp::socket, GroupWorker &);
	WorkerSession(tcp::socket, GroupWorker &, SessionID ID, GroupID _groupID);
	WorkerSession(tcp::socket, GroupWorker &, SessionID ID, GroupID _groupID, Log_ptr &);

	int handle_message();

	void start();

	bool send_response_string();
	bool send_test_string();

	bool send_matrix_blocks();
	bool receive_matrix_blocks();

	void handle_send_indexed_rows();
	void handle_request_indexed_rows();

	// -------------------------------------   Matrix Management   -----------------------------------

	//	MatrixHandle register_matrix(size_t num_rows, size_t num_cols);


	void remove_session();

	std::shared_ptr<WorkerSession> shared_from_this()
	{
		return std::static_pointer_cast<WorkerSession>(Session::shared_from_this());
	}

private:

	GroupWorker & group_worker;


	// ---------------------------------------   Information   ---------------------------------------



	// -----------------------------------------   Testing   -----------------------------------------

//	int receive_test_string(const char *, const uint32_t);
//	int send_test_string();

};

typedef std::shared_ptr<WorkerSession> WorkerSession_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__WORKERSESSION_HPP
