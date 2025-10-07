/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 12:59:07 by yanli             #+#    #+#             */
/*   Updated: 2025/10/04 13:44:54 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include "HttpRequest.hpp"
#include "LocationConfig.hpp"
#include "EventLoop.hpp"
#include "utility.hpp"
#include "CodePage.hpp"
#include "timestring.hpp"
#include "Connection.hpp"

CgiHandler::CgiHandler(const HttpRequest &req, const std::string &cgi,
const std::string &script, const LocationConfig *loc, Connection *owner)
: _pid(-1), _output(), _body(req.getBody()), _body_sent(0), _headers_parsed(false),
  _done(false), _status(200), _headers(), _response_body(),
  _body_start(0), _loop(NULL),
  _start(std::time(NULL)), _cgi_path(cgi), _script(script), _workdir(), _env(), _owner(owner)
{
	_stdin_pipe[0] = _stdin_pipe[1] = -1;
	_stdout_pipe[0] = _stdout_pipe[1] = -1;
	if (!_cgi_path.empty())
	{
		std::string	fullpath = expandPath(_cgi_path);
		if (!fullpath.empty() && fullpath != ".")
			_cgi_path = fullpath;
	}
	_env = req.toCgiEnvironment();
#ifdef	_DEBUG
	std::map<std::string, std::string>::const_iterator cl = _env.find("CONTENT_LENGTH");
	if (cl != _env.end())
	std::cerr << "DEBUG: CGI ENV CONTENT_LENGTH=" << cl->second << std::endl;
	else
		std::cerr << "DEBUG: CGI ENV has no CONTENT_LENGTH" << std::endl;
	std::map<std::string, std::string>::const_iterator secret = _env.find("HTTP_X_SECRET_HEADER_FOR_TEST");
	if (secret != _env.end())
		std::cerr << "DEBUG: CGI ENV HTTP_X_SECRET_HEADER_FOR_TEST=" << secret->second << std::endl;
	else
		std::cerr << "DEBUG: CGI ENV missing HTTP_X_SECRET_HEADER_FOR_TEST" << std::endl;
	std::cerr<<"The path of the script being sent to cgi is: "<<script<<std::endl;
	int	fd = ::open(script.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
	if (fd < 0)
		std::cerr<<"DEBUG: unable to open script for peek: "<<script<<std::endl;
	else
	{
		char	buf[2] = {0, 0};
		ssize_t	l = ::read(fd, buf, 1);
		if (l < 0)
			std::cerr<<"DEBUG: initial read on script failed"<<std::endl;
		int r = ::write(STDERR_FILENO, buf, 1);
		(void)r;
		close(fd);
	}
#endif

	std::string script_real = expandPath(script);
	if (script_real.empty() || script_real == ".")
		script_real = script;
	if (!req.getPath().empty())
		_env["PATH_INFO"] = req.getPath();
	else
		_env["PATH_INFO"] = "/";
	_env["SCRIPT_FILENAME"] = script_real;
	_env["PATH_TRANSLATED"] = script_real;
	_env["SCRIPT_NAME"] = req.getPath();
	_env["SERVER_SOFTWARE"] = "Webserv/1.0";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["AUTH_TYPE"] = "";
	_env["REDIRECT_STATUS"] = "200";  // Required for PHP CGI security

	std::string	content_type = req.getHeader("Content-Type");
	if (!content_type.empty())
		_env["CONTENT_TYPE"] = content_type;
	std::string	request_uri = req.getPath();
	if (!req.getQuery().empty())
		request_uri += "?" + req.getQuery();
	_env["REQUEST_URI"] = request_uri;
	
	if (loc)
	{
		_workdir = loc->getAlias();

		// Extract just the filename since we'll cd to the script directory
		size_t	last_slash = script.find_last_of('/');
		if (last_slash != std::string::npos)
			_script = script.substr(last_slash + 1);
		else
			_script = script;
	}
}

CgiHandler::~CgiHandler(void)
{
	removeFromEventLoop();
	closePipes();
	if (_pid > 0)
	{
		int		status = 0;
		pid_t	r = ::waitpid(_pid, &status, WNOHANG);
		if (r == 0)
		{
			(void)::kill(_pid, SIGTERM);
			(void)::waitpid(_pid, &status, WNOHANG);
			(void)::kill(_pid, SIGKILL);
		}
		_pid = -1;
	}
}

void	CgiHandler::closePipes(void)
{
	for (int i = 0; i < 2; i++)
	{
		if (_stdin_pipe[i] > -1)
		{
			(void)::close(_stdin_pipe[i]);
			_stdin_pipe[i] = -1;
		}
		if (_stdout_pipe[i] > -1)
		{
			(void)::close(_stdout_pipe[i]);
			_stdout_pipe[i] = -1;
		}
	}
}

char	**CgiHandler::buildEnv(void) const
{
	try
	{
		char	**envp = new char*[_env.size() + 1];
		size_t	i = 0;

		for (std::map<std::string, std::string>::const_iterator it = _env.begin();
			it != _env.end(); ++it, ++i)
		{
			std::string	line = it->first + "=" + it->second;
			envp[i] = new char[line.size() + 1];
			std::strcpy(envp[i], line.c_str());
		}
		envp[i] = NULL;
		return envp;
	}
	catch (const std::exception &e)
	{
		std::cerr<<"buildEnv fucked up"<<e.what()<<std::endl;
		return (NULL);
	}
	catch (...)
	{
		std::cerr<<"Non-standard exception caught"<<std::endl;
		return (NULL);
	}
}

void	CgiHandler::freeEnv(char **envp) const
{
	if (!envp)
		return;
	for (size_t i = 0; envp[i]; ++i)
	{
		delete[] envp[i];
		envp[i] = NULL;
	}
	delete[] envp;
	envp = NULL;
}

bool	CgiHandler::execute(EventLoop &loop)
{
	_loop = &loop;

	if (::pipe(_stdin_pipe) < 0)
	{
		std::cerr<<"pipe fucked up"<<std::strerror(errno)<<std::endl;
		return (false);
	}
	(void)::set_nonblock_fd(_stdin_pipe[1]);
	if (::pipe(_stdout_pipe) < 0)
	{
		(void)::close(_stdin_pipe[0]);
		(void)::close(_stdin_pipe[1]);
		std::cerr<<"pipe fucked up"<<std::strerror(errno)<<std::endl;
		return (false);
	}

	(void)set_nonblock_fd(_stdout_pipe[0], "CgiHandler.cpp:197");
	(void)set_cloexec_fd(_stdin_pipe[0], "CgiHandler.cpp:199");
	(void)set_cloexec_fd(_stdout_pipe[0], "CgiHandler.cpp:200");
	(void)set_cloexec_fd(_stdin_pipe[1], "CgiHandler.cpp:201");
	(void)set_cloexec_fd(_stdout_pipe[1], "CgiHandler.cpp:202");


	_pid = ::fork();
	if (_pid < 0)
	{
		std::cerr<<"fork fucked up"<<std::strerror(errno)<<std::endl;
		closePipes();
		return false;
	}
	else if (_pid == 0)
	{
		// Child
		// Make child FD blccking so we can detect problems earlier !
		int	flags = ::fcntl(_stdin_pipe[0], F_GETFL);
		if (flags >= 0)
			(void)::fcntl(_stdin_pipe[0], F_SETFL, flags & ~O_NONBLOCK);
		flags = ::fcntl(_stdout_pipe[1], F_GETFL);
		if (flags >= 0)
			(void)::fcntl(_stdout_pipe[1], F_SETFL, flags & ~O_NONBLOCK);

		if (!_workdir.empty())
		{
			if (::chdir(_workdir.c_str()) < 0)
			{
				std::cerr<<"CFG: chdir failed: "<<std::strerror(errno)<<std::endl;
				std::exit(126);
			}
		}

		if (::dup2(_stdin_pipe[0], STDIN_FILENO) < 0 
			|| ::dup2(_stdout_pipe[1], STDOUT_FILENO) < 0)
		{
			std::cerr<<"dup2 fucked up"<<std::strerror(errno)<<std::endl;
			std::exit(126);
		}

		(void)::close(_stdin_pipe[0]);
		(void)::close(_stdin_pipe[1]);
		(void)::close(_stdout_pipe[0]);
		(void)::close(_stdout_pipe[1]);

		char	*argv[3];

		std::cerr<<"CGI executable path: "<<_cgi_path<<std::endl;
		argv[0] = const_cast<char*>(_cgi_path.c_str());
		argv[1] = const_cast<char*>(_script.c_str());
		argv[2] = NULL;

		char	**envp = buildEnv();
		::execve(_cgi_path.c_str(), argv, envp);
		std::cerr<<"\n---EXECVE RETURNED ! CGI FUCKED UP ! "<<std::strerror(errno)<<std::endl;
		freeEnv(envp);
		std::exit(1);
	}
	else
	{// Parent
		(void)::close(_stdin_pipe[0]);
		_stdin_pipe[0] = -1;
		(void)::close(_stdout_pipe[1]);
		_stdout_pipe[1] = -1;

		if (!_body.empty())
			_loop->add(_stdin_pipe[1], EVENT_WRITE, this);
		else
		{
			(void)::close(_stdin_pipe[1]);
			_stdin_pipe[1] = -1;
		}
		_loop->add(_stdout_pipe[0], EVENT_READ, this);
	}
	return true;
}

void	CgiHandler::onReadable(int fd)
{
	if (fd != _stdout_pipe[0])
		return;

	char	buf[655360];
	ssize_t	n = ::read(fd, buf, sizeof(buf));

	if (n > 0)
	{
		_output.append(buf, static_cast<size_t>(n));
#ifdef	_DEBUG_CGI
std::cerr<<"DEBUG: CGI stdout read "<<n
<<" bytes (total "<<_output.size()<<")"<<std::endl;
#endif
		if (!_headers_parsed)
			parseHeaders();
		return;
	}
	if (n < 0)
	{
		if (_loop)
			_loop->remove(fd);
		(void)::close(_stdout_pipe[0]);
		_stdout_pipe[0] = -1;
		_done = true;
		return;
	}

	if (_loop)
		_loop->remove(fd);
	(void)::close(_stdout_pipe[0]);
	_stdout_pipe[0] = -1;

	if (!_headers_parsed)
		parseHeaders();
	if (_headers_parsed)
		_response_body = _output.substr(_body_start);
#ifdef	_DEBUG
	std::cerr<<"DEBUG: CGI output size: "<<_output.size()<<std::endl;
#endif
	if (_pid > 0)
	{
		int		status = 0;
		pid_t	r = ::waitpid(_pid, &status, WNOHANG);
		if (r == _pid || r < 0)
			_pid = -1;
	}
	_done = true;
}

void	CgiHandler::onWritable(int fd)
{
	if (fd != _stdin_pipe[1])
		return;

	if (_body_sent >= _body.size())
	{
		if (_loop)
			_loop->remove(fd);
		if (_stdin_pipe[1] != -1)
		{
			(void)::close(_stdin_pipe[1]); // EOF to CGI's stdin
			_stdin_pipe[1] = -1;
		}
		return;
	}

	ssize_t n = ::write(fd, _body.data() + _body_sent,
		static_cast<size_t>(_body.size() - _body_sent));
	if (n > 0)
	{
		_body_sent += static_cast<size_t>(n);
#ifdef	_DEBUG_CGI
std::cerr<<"DEBUG: CGI stdin wrote "<<n
<<" bytes (total "<<_body_sent<<")"<<std::endl;
#endif
		if (_body_sent >= _body.size())
		{
			if (_loop)
				_loop->remove(fd);
			if (_stdin_pipe[1] != -1)
			{
				(void)::close(_stdin_pipe[1]);
				_stdin_pipe[1] = -1;
			}
		}
		return;
	}
	if (!n)
		return;
	if (_loop)
		_loop->remove(fd);
	if (_stdin_pipe[1] != -1)
	{
		(void)::close(_stdin_pipe[1]);
		_stdin_pipe[1] = -1;
	}
}

void 	CgiHandler::onError(int fd)
{
	(void)fd;
	removeFromEventLoop();
	closePipes();
	if (_pid > 0)
	{
		(void)::waitpid(_pid, NULL, WNOHANG);
		(void)::kill(_pid, SIGKILL);
		_pid = -1;
	}
	_done = true;
}

void	CgiHandler::onHangup(int fd)
{
	(void)fd;
#ifdef _DEBUG
	std::cerr << "DEBUG: CGI onHangup" << std::endl;
#endif
	if (_loop && _stdout_pipe[0] != -1)
		_loop->remove(_stdout_pipe[0]);
	if (_stdout_pipe[0] != -1)
	{
		char	buf[65536];
		while (true)
		{
			ssize_t n = ::read(_stdout_pipe[0], buf, sizeof(buf));
			if (n > 0)
			{
				_output.append(buf, static_cast<size_t>(n));
				if (static_cast<size_t>(n) < sizeof(buf))
					break;
				continue;
			}
			break;
		}
		(void)::close(_stdout_pipe[0]);
		_stdout_pipe[0] = -1;
	}
	if (!_headers_parsed)
		parseHeaders();
	if (_headers_parsed)
		_response_body = _output.substr(_body_start);
	if (_pid > 0)
	{
		int	status = 0;
		pid_t	r = ::waitpid(_pid, &status, WNOHANG);
		if (r != _pid)
			(void)::kill(_pid, SIGKILL);
		_pid = -1;
#ifdef	_DEBUG
		if (r < 0)
		{
			std::cerr<<"waitpid fucked up"<<std::strerror(errno)<<std::endl;
		}
		if (WIFEXITED(status))
			std::cerr<<"cgi exited with "<<WEXITSTATUS(status)<<std::endl;
		if (WIFSIGNALED(status))
			std::cerr<<"cgi recived signal "<<WTERMSIG(status)<<std::endl;
#endif
	}
	_done = true;
	if (_owner)
		_owner->markCgiReady();
	if (_loop)
		_loop->notify();
}

void	CgiHandler::onTick(int fd)
{
	(void)fd;
	
	if (isTimeout())
	{
		removeFromEventLoop();
		closePipes();
		if (_pid > 0)
		{
			(void)::waitpid(_pid, NULL, WNOHANG);
			(void)::kill(_pid, SIGKILL);
			_pid = -1;
		}
		_done = true;
	}
}

bool	CgiHandler::parseHeaders(void)
{
	std::string::size_type	pos = _output.find("\r\n\r\n");
	bool	crlf = true;

	if (pos == std::string::npos)
	{
		pos = _output.find("\n\n");
		crlf = false;
		if (pos == std::string::npos)
			return false;
	}

	_headers_parsed = true;
	std::string	head = _output.substr(0, pos);
	_body_start = pos + (crlf ? 4 : 2);
	_response_body = _output.substr(_body_start);

	std::istringstream	iss(head);
	std::string			line;

	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		std::string::size_type colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string	key = trim(line.substr(0, colon));
		std::string	val = trim(line.substr(colon + 1));

		if (toLower(key) == "status")
		{
			std::istringstream	ss(val);
			int					code = 0;
			ss >> code;
			if (code >= 100 && code <= 999)
				_status = code;
		}
		else
			_headers[key] = val;
	}
	return (true);
}

bool CgiHandler::isDone(void) const
{
	return _done;
}

bool CgiHandler::isTimeout(void) const
{
	return (std::time(NULL) - _start) > 300;
}

std::string	CgiHandler::getResponse(void) const
{
	std::ostringstream oss;

	oss << "HTTP/1.1 " << _status << " "<<CodePage::getInstance().getReason(_status)<< "\r\nDate: "<<getTimeString()<<"\r\n";

		// Only inject default Content-Type if CGI didn't send one.
	if (_headers.find("Content-Type") == _headers.end()
	    && _headers.find("content-type") == _headers.end())
			oss << "Content-Type: text/html\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: CGI header " << it->first << ": " << it->second << std::endl;
#endif
		oss << it->first << ": " << it->second << "\r\n";
	}
	if (_headers.find("Content-Length") == _headers.end())
		oss << "Content-Length: " << _response_body.size() << "\r\n";

	oss << "\r\n" << _response_body;
	return (oss.str());
}

void	CgiHandler::removeFromEventLoop(void)
{
	if (!_loop)
		return;
	if (_stdout_pipe[0] != -1)
		_loop->remove(_stdout_pipe[0]);
	if (_stdin_pipe[1] != -1)
		_loop->remove(_stdin_pipe[1]);
}
