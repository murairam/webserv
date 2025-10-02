/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/01 15:53:56 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/10/02 13:34:52 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"
#include "HttpRequest.hpp"
#include "LocationConfig.hpp"
#include "EventLoop.hpp"
#include "utility.hpp"

CgiHandler::CgiHandler(const HttpRequest &req, const std::string &cgi,
					   const std::string &script, const LocationConfig *loc)
: _pid(-1), _output(), _body(req.getBody()), _body_sent(0), _headers_parsed(false),
  _done(false), _status(200), _headers(), _response_body(), _loop(NULL),
  _start(std::time(NULL)), _cgi_path(cgi), _script(script), _workdir(), _env()
{
	_stdin_pipe[0] = _stdin_pipe[1] = -1;
	_stdout_pipe[0] = _stdout_pipe[1] = -1;

	_env = req.toCgiEnvironment();

	_env["PATH_INFO"] = script;
	_env["SCRIPT_FILENAME"] = script;
	_env["SCRIPT_NAME"] = script;
	_env["SERVER_SOFTWARE"] = "Webserv/1.0";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["REDIRECT_STATUS"] = "200";  // Required for PHP CGI security

	if (loc)
	{
		_workdir = loc->getAlias();

		// Extract just the filename since we'll cd to the script directory
		size_t last_slash = script.find_last_of('/');
		if (last_slash != std::string::npos)
			_script = script.substr(last_slash + 1);
		else
			_script = script;
	}
}

CgiHandler::~CgiHandler(void)
{
	closePipes();
	if (_pid > 0)
	{
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
	}
}

void CgiHandler::closePipes(void)
{
	for (int i = 0; i < 2; i++)
	{
		if (_stdin_pipe[i] > -1)
		{
			close(_stdin_pipe[i]);
			_stdin_pipe[i] = -1;
		}
		if (_stdout_pipe[i] > -1)
		{
			close(_stdout_pipe[i]);
			_stdout_pipe[i] = -1;
		}
	}
}

char **CgiHandler::buildEnv(void) const
{
	char **envp = new char*[_env.size() + 1];
	size_t i = 0;

	for (std::map<std::string, std::string>::const_iterator it = _env.begin();
		 it != _env.end(); ++it, ++i)
	{
		std::string line = it->first + "=" + it->second;
		envp[i] = new char[line.size() + 1];
		std::strcpy(envp[i], line.c_str());
	}
	envp[i] = NULL;
	return envp;
}

void CgiHandler::freeEnv(char **envp) const
{
	if (!envp)
		return;
	for (size_t i = 0; envp[i]; ++i)
		delete[] envp[i];
	delete[] envp;
}

bool CgiHandler::execute(EventLoop &loop)
{
	_loop = &loop;

	if (pipe(_stdin_pipe) < 0 || pipe(_stdout_pipe) < 0)
		return false;

	set_nonblock_fd(_stdin_pipe[1]);
	set_nonblock_fd(_stdout_pipe[0]);

	_pid = fork();
	if (_pid < 0)
	{
		closePipes();
		return false;
	}

	if (_pid == 0)
	{
		// Child
		if (!_workdir.empty())
			chdir(_workdir.c_str());

		dup2(_stdin_pipe[0], STDIN_FILENO);
		dup2(_stdout_pipe[1], STDOUT_FILENO);

		close(_stdin_pipe[0]);
		close(_stdin_pipe[1]);
		close(_stdout_pipe[0]);
		close(_stdout_pipe[1]);

		char *argv[3];
		argv[0] = const_cast<char*>(_cgi_path.c_str());
		argv[1] = const_cast<char*>(_script.c_str());
		argv[2] = NULL;

		char **envp = buildEnv();
		execve(_cgi_path.c_str(), argv, envp);
		exit(1);
	}

	// Parent
	close(_stdin_pipe[0]);
	_stdin_pipe[0] = -1;
	close(_stdout_pipe[1]);
	_stdout_pipe[1] = -1;

	if (!_body.empty())
		_loop->add(_stdin_pipe[1], EVENT_WRITE, this);
	else
	{
		close(_stdin_pipe[1]);
		_stdin_pipe[1] = -1;
	}

	_loop->add(_stdout_pipe[0], EVENT_READ, this);
	return true;
}

void CgiHandler::onReadable(int fd)
{
	if (fd != _stdout_pipe[0])
		return;

	char buf[8192];
	ssize_t n = read(fd, buf, sizeof(buf));

	if (n > 0)
	{
		// Safety check: prevent output buffer from growing too large
		if (_output.size() + n > 50 * 1024 * 1024) // 50MB limit
		{
			// CGI output too large - terminate the process
			if (_pid > 0)
			{
				kill(_pid, SIGKILL);
				waitpid(_pid, NULL, 0);
				_pid = -1;
			}
			close(fd);
			_stdout_pipe[0] = -1;
			_done = true;
			_status = 500; // Internal Server Error
			return;
		}

		_output.append(buf, n);
		if (!_headers_parsed)
			parseHeaders();
	}
	else if (n == 0)
	{
		// Don't remove from loop here - let Connection handle it
		close(fd);
		_stdout_pipe[0] = -1;

		if (!_headers_parsed)
			parseHeaders();

		if (_pid > 0)
		{
			waitpid(_pid, NULL, 0);
			_pid = -1;
		}
		_done = true;
	}
}

void CgiHandler::onWritable(int fd)
{
	if (fd != _stdin_pipe[1])
		return;

	if (_body_sent >= _body.size())
	{
		if (_loop)
			_loop->remove(fd);
		close(fd);
		_stdin_pipe[1] = -1;
		return;
	}

	ssize_t n = write(fd, _body.data() + _body_sent, _body.size() - _body_sent);
	if (n > 0)
		_body_sent += n;
}

void CgiHandler::onError(int fd)
{
	(void)fd;
	_done = true;
}

void CgiHandler::onHangup(int fd)
{
	(void)fd;
	_done = true;
}

void CgiHandler::onTick(int fd)
{
	(void)fd;
	if (isTimeout())
	{
		_done = true;
		if (_pid > 0)
		{
			kill(_pid, SIGKILL);
			waitpid(_pid, NULL, 0);
			_pid = -1;
		}
	}
}

bool CgiHandler::parseHeaders(void)
{
	std::string::size_type pos = _output.find("\r\n\r\n");
	bool crlf = true;

	if (pos == std::string::npos)
	{
		pos = _output.find("\n\n");
		crlf = false;
		if (pos == std::string::npos)
			return false;
	}

	_headers_parsed = true;
	std::string head = _output.substr(0, pos);
	_response_body = _output.substr(pos + (crlf ? 4 : 2));

	std::istringstream iss(head);
	std::string line;

	while (std::getline(iss, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		std::string::size_type colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = trim(line.substr(0, colon));
		std::string val = trim(line.substr(colon + 1));

		if (toLower(key) == "status")
		{
			std::istringstream ss(val);
			ss >> _status;
		}
		else
			_headers[key] = val;
	}
	return true;
}

bool CgiHandler::isDone(void) const
{
	return _done;
}

bool CgiHandler::isTimeout(void) const
{
	return (std::time(NULL) - _start) > 30;
}

std::string CgiHandler::getResponse(void) const
{
	std::ostringstream oss;

	oss << "HTTP/1.1 " << _status << " ";
	if (_status == 200)
		oss << "OK";
	else if (_status == 404)
		oss << "Not Found";
	else if (_status == 500)
		oss << "Internal Server Error";
	else
		oss << "Status";
	oss << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		 it != _headers.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";

	if (_headers.find("Content-Length") == _headers.end())
		oss << "Content-Length: " << _response_body.size() << "\r\n";

	oss << "\r\n" << _response_body;
	return oss.str();
}

void CgiHandler::removeFromEventLoop(void)
{
	if (!_loop)
		return;

	if (_stdout_pipe[0] != -1) {
		_loop->remove(_stdout_pipe[0]);
	}
	if (_stdin_pipe[1] != -1) {
		_loop->remove(_stdin_pipe[1]);
	}
}
