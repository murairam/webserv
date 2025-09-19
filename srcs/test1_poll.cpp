# include "test1_poll.hpp"
#include <cerrno>
#include <fcntl.h>

static void set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags >= 0)
	{
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}
}

// ===== RequestParser (stub you fill with your logic) =====

RequestParser::RequestParser() {}
RequestParser::RequestParser(const RequestParser &other) { (void)other; }
RequestParser &RequestParser::operator=(const RequestParser &other)
{
	(void)other; return *this;
}
RequestParser::~RequestParser() {}

bool RequestParser::parse(std::istream &in, std::string &out_payload)
{
	// Example: echo a minimal response; replace with your real parser.
	std::string dump;
	std::string line;
	while (std::getline(in, line))
	{
		dump += line;
		dump += "\n";
	}
	out_payload =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 12\r\n"
		"Connection: close\r\n"
		"\r\n"
		"Hello world";
	return true;
}

// ===== Connection =====

Connection::Connection() : _fd(-1), _want_close(false) {}
Connection::Connection(int fd) : _fd(fd), _want_close(false) {}
Connection::Connection(const Connection &o)
: _fd(o._fd), _in_buf(o._in_buf), _out_buf(o._out_buf), _want_close(o._want_close) {}
Connection &Connection::operator=(const Connection &o)
{
	if (this != &o)
	{
		_fd = o._fd;
		_in_buf = o._in_buf;
		_out_buf = o._out_buf;
		_want_close = o._want_close;
	}
	return *this;
}
Connection::~Connection() {}

int Connection::getFd() const { return _fd; }
std::string &Connection::inBuf() { return _in_buf; }
std::string &Connection::outBuf() { return _out_buf; }
bool Connection::wantClose() const { return _want_close; }
void Connection::setWantClose(bool v) { _want_close = v; }

// ===== PollReactor =====

PollReactor::PollReactor() : _listen_fd(-1) {}
PollReactor::PollReactor(int listen_fd) : _listen_fd(listen_fd)
{
	if (_listen_fd >= 0) set_nonblocking(_listen_fd);
}
PollReactor::PollReactor(const PollReactor &o)
: _listen_fd(o._listen_fd), _conns(o._conns), _pfds(o._pfds), _parser(o._parser) {}
PollReactor &PollReactor::operator=(const PollReactor &o)
{
	if (this != &o)
	{
		_listen_fd = o._listen_fd;
		_conns = o._conns;
		_pfds = o._pfds;
		_parser = o._parser;
	}
	return *this;
}
PollReactor::~PollReactor() {}

void PollReactor::addClient(int fd)
{
	set_nonblocking(fd);
	_conns.insert(std::make_pair(fd, Connection(fd)));
}

void PollReactor::rebuildPollfds()
{
	_pfds.clear();

	if (_listen_fd >= 0)
	{
		struct pollfd p;
		p.fd = _listen_fd;
		p.events = POLLIN;
		p.revents = 0;
		_pfds.push_back(p);
	}

	std::map<int, Connection>::iterator it = _conns.begin();
	while (it != _conns.end())
	{
		struct pollfd p;
		p.fd = it->first;
		p.events = POLLIN;
		if (!it->second.outBuf().empty())
			p.events |= POLLOUT;
		p.revents = 0;
		_pfds.push_back(p);
		++it;
	}
}

void PollReactor::handleAccept()
{
	if (_listen_fd < 0) return;

	for (;;)
	{
		int cfd = accept(_listen_fd, NULL, NULL);
		if (cfd < 0) return; // no errno checks; poll gated us
		addClient(cfd);
	}
}

void PollReactor::handleRead(Connection &c)
{
	char tmp[4096];
	int n = recv(c.getFd(), tmp, 4096, 0);
	if (n > 0)
	{
		c.inBuf().append(tmp, n);
		return;
	}
	if (n == 0)
	{
		c.setWantClose(true);
		return;
	}
	// n < 0: treat as temporary/unrecoverable; close to keep server robust.
	c.setWantClose(true);
}

void PollReactor::handleMaybeParse(Connection &c)
{
	// Detect a complete HTTP request in _in_buf.
	// Minimal demo: headers end at \r\n\r\n, and either no body,
	// or Content-Length bytes available. Replace with your robust logic.

	std::string &buf = c.inBuf();
	std::string::size_type pos = buf.find("\r\n\r\n");
	if (pos == std::string::npos) return;

	// For demo, parse whole buffer as one request.
	std::istringstream iss(buf);
	std::string response;
	if (_parser.parse(iss, response))
	{
		c.outBuf().append(response);
		c.setWantClose(true); // demo sends Connection: close
		buf.erase(); // consumed
	}
}

void PollReactor::handleWrite(Connection &c)
{
	if (c.outBuf().empty()) return;

	int n = send(c.getFd(), c.outBuf().data(),
	             (int)c.outBuf().size(), 0);
	if (n > 0)
	{
		c.outBuf().erase(0, n);
		return;
	}
	// n <= 0: close to keep invariants (do not read errno).
	c.setWantClose(true);
}

void PollReactor::drop(int fd)
{
	std::map<int, Connection>::iterator it = _conns.find(fd);
	if (it != _conns.end())
	{
		close(fd);
		_conns.erase(it);
	}
}

void PollReactor::runOnce(int timeout_ms)
{
	rebuildPollfds();

	if (_pfds.empty()) return;

	int rc = poll(&_pfds[0], (int)_pfds.size(), timeout_ms);
	if (rc <= 0) return;

	std::size_t i = 0;
	while (i < _pfds.size())
	{
		struct pollfd &p = _pfds[i];

		if (p.fd == _listen_fd)
		{
			if (p.revents & POLLIN) handleAccept();
			++i;
			continue;
		}

		std::map<int, Connection>::iterator it = _conns.find(p.fd);
		if (it == _conns.end())
		{
			++i;
			continue;
		}

		Connection &c = it->second;

		if (p.revents & POLLIN) handleRead(c);

		if (!c.wantClose())
			handleMaybeParse(c);

		if (p.revents & POLLOUT)
			handleWrite(c);

		if (c.wantClose() && c.outBuf().empty())
			drop(p.fd);

		++i;
	}
}
