#ifndef ALCHEMIST__SESSION_HPP
#define ALCHEMIST__SESSION_HPP


#include "Alchemist.hpp"
#include "Message.hpp"
//#include "data_stream.hpp"
#include "Server.hpp"
#include "LibraryManager.hpp"
#include "utility/logging.hpp"


namespace alchemist {

using boost::asio::ip::tcp;
using std::string;

typedef uint16_t Session_ID;
typedef std::deque<Message> Message_queue;

class Server;

class Session : public std::enable_shared_from_this<Session>
{
public:
	Session(tcp::socket, Server &);
	Session(tcp::socket, Server &, uint16_t);
	Session(tcp::socket, Server &, uint16_t, Log_ptr &);
	virtual ~Session() { };

	virtual void start() = 0;
	virtual void remove_session() = 0;
	virtual int handle_message() = 0;

	void set_log(Log_ptr _log);
	void set_ID(Session_ID _ID);
	void set_admin_privilege(bool privilege);

	Session_ID get_ID() const;
	string get_address() const;
	uint16_t get_port() const;
	bool get_admin_privilege() const;

	bool send_handshake();
	bool check_handshake();

	virtual bool send_response_string() = 0;
	bool send_test_string();

	bool load_library();
	bool run_task();
	bool unload_library();

	void wait();

	void write_string(const string & data);
	void write_unsigned_char(const unsigned char & data);
	void write_uint16(const uint16_t & data);
	void write_uint32(const uint32_t & data);

	void write(const char * data, std::size_t length, datatype dt);

	void read_header();
	void read_body();
	void flush();

//	void assign_workers();
//	bool list_all_workers();
//	bool list_active_workers();
//	bool list_inactive_workers();
//	bool list_assigned_workers();

	Message & new_message();

protected:
	bool admin_privilege;
	bool ready;
	Log_ptr log;
	LibraryManager lm;

	tcp::socket socket;
	Server & server;
	Message read_msg;
	Message write_msg;
	Message_queue write_msgs;

	Session_ID ID;
	string address = "";
	uint16_t port = 0;
};

typedef std::shared_ptr<Session> Session_ptr;

}			// namespace alchemist

#endif		// ALCHEMIST__SESSION_HPP
