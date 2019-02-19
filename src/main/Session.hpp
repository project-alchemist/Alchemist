#ifndef ALCHEMIST__SESSION_HPP
#define ALCHEMIST__SESSION_HPP

#include "Message.hpp"

namespace alchemist {

class Server;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket);
	Session(tcp::socket, SessionID ID, ClientID _clientID);
	Session(tcp::socket, SessionID ID, ClientID _clientID, Log_ptr &);
	virtual ~Session() { };

	vector<WorkerID>  allocated_workers;
	vector<LibraryID> loaded_libraries;
	vector<TaskID> tasks;

	virtual void start() = 0;
	virtual void remove_session() = 0;
	virtual int handle_message() = 0;

	void set_log(Log_ptr _log);
	void setID(SessionID ID);
	void set_admin_privilege(bool privilege);

	SessionID get_sessionID() const;
	string get_address() const;
	uint16_t get_port() const;
	bool get_admin_privilege() const;

	bool handle_handshake();
	bool valid_handshake();
	bool invalid_handshake();

	virtual bool send_response_string() = 0;
	bool send_test_string();

	void wait();

	void write(const char * data, std::size_t length, datatype dt);

	void read_header();
	void read_body();
	void flush();

	string preamble();
	string client_preamble();
	string session_preamble();
	string address_preamble();

	SessionID assign_sessionID();

	void set_client_language(client_language _cl);

//	void assign_workers();
//	bool list_all_workers();
//	bool list_active_workers();
//	bool list_inactive_workers();
//	bool list_assigned_workers();

	Message & new_message();

	Message read_msg;
	Message write_msg;
protected:
	ClientID clientID;
	SessionID sessionID;

	bool admin_privilege;
	bool ready;
	Log_ptr log;

	client_language cl;

	tcp::socket socket;

//	Message_queue write_msgs;

	string address = "";
	uint16_t port = 0;
};

typedef std::shared_ptr<Session> Session_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__SESSION_HPP
