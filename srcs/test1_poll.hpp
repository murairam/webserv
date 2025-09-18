#ifndef POLLREACTOR_HPP
#define POLLREACTOR_HPP

#include <map>
#include <string>
#include <vector>
#include <sstream>     // for std::istringstream
#include <poll.h>      // poll()
#include <unistd.h>    // close()
#include <sys/socket.h>// recv(), send()

class RequestParser
{
public:
	RequestParser();
	RequestParser(const RequestParser &other);
	RequestParser &operator=(const RequestParser &other);
	~RequestParser();

	// Your existing parser: source-agnostic now.
	// Reads headers/body from 'in' and fills 'out_payload' with response bytes.
	// Return true if parsing succeeded and a response is ready.
	bool parse(std::istream &in, std::string &out_payload);
};

class Connection
{
private:
	int         _fd;            // socket file descriptor for this client
	std::string _in_buf;        // bytes accumulated from recv()
	std::string _out_buf;       // bytes to send via send()
	bool        _want_close;    // mark connection to be closed after flush

public:
	Connection();
	Connection(int fd);
	Connection(const Connection &other);
	Connection &operator=(const Connection &other);
	~Connection();

	int         getFd() const;
	std::string &inBuf();
	std::string &outBuf();
	bool        wantClose() const;
	void        setWantClose(bool v);
};

class PollReactor
{
private:
	int                          _listen_fd;   // listening socket fd
	std::map<int, Connection>    _conns;       // map: fd -> connection state
	std::vector<struct pollfd>   _pfds;        // poll() watched fds
	RequestParser                _parser;      // request parser instance

private:
	void rebuildPollfds();                     // rebuild _pfds from _conns
	void handleAccept();                       // accept new client(s)
	void handleRead(Connection &c);            // read from client into _in_buf
	void handleMaybeParse(Connection &c);      // if a full request is ready, parse it
	void handleWrite(Connection &c);           // flush _out_buf to socket
	void drop(int fd);                         // close and erase connection

public:
	PollReactor();
	PollReactor(int listen_fd);
	PollReactor(const PollReactor &other);
	PollReactor &operator=(const PollReactor &other);
	~PollReactor();

	// Run one iteration: one call to poll(), then handle events.
	void runOnce(int timeout_ms);

	// Add a pre-accepted client (optional helper).
	void addClient(int fd);
};

#endif
