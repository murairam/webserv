/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/21 20:07:49 by yanli             #+#    #+#             */
/*   Updated: 2025/09/24 22:11:33 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "EventLoop.hpp"

Connection::~Connection(void)
{
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

Connection::Connection
(int fd, const std::string &server_name, const ServerConfig *server)
:_fd(fd), _loop(0), _server_name(server_name), _inbuf(),
_outbuf(), _engaged(false), _should_close(false), _server(server),
_method(0), r(), _header(), _body()
{
	(void)set_nonblock_fd_nothrow(_fd);
}

void	Connection::engageLoop(EventLoop &loop)
{
	int	events = EVENT_READ;
	
	if (_fd < 0)
		return;
	_loop = &loop;
	if (!_outbuf.empty())
		events = events | EVENT_WRITE;
	_loop->add(_fd, events, this);
	_engaged = true;
}

void	Connection::sendResponse(Response &reply)
{
	reply.setHeader(std::string("Connection"), std::string("close"));
	queueWrite(reply.serialize());
}

void	Connection::sendErrPage(int code)
{
	Response	reply;
	if (_server && !_server->getErrorPage(code).empty())
		reply = Response::createErrorResponse(code, _server->getErrorPage(code));
	else
		reply = Response::createErrorResponse(code);
	r._persistent = false;
	sendResponse(reply);
	r._job_done = true;
	_should_close = true;
	_inbuf.clear();
}

void	Connection::parseGET(void)
{
	std::istringstream	iss(_header);
	std::string			line;
	std::string			keyword;
	std::string			valueword;
	bool				toggle = true;

	if (r._job_done)
		return;
	while (std::getline(iss, line))
	{
		keyword.clear();
		valueword.clear();
		if (!line.empty() && line[0] == '\r')
			return ;
		iss>>keyword;
#ifdef	_DEBUG
		std::cout<<line<<std::endl;
		std::cout<<"keyword being evaluated is:'"<<keyword<<"'"<<std::endl;
#endif
		if (toggle)
		{
			toggle = false;
			if (keyword.empty())
				goto reject_400;
			std::string::size_type	query_mark_pos = keyword.find('?');
			if (query_mark_pos != std::string::npos)
			{
				std::string	target = keyword.substr(0, query_mark_pos);
				if (query_mark_pos + 1 >= keyword.size())
				{
					r._target = target;
					r._target_set = true;
					continue;
				}
				std::string	query = keyword.substr(query_mark_pos + 1, keyword.size());
				r._target = target;
				r._target_set = true;
				r._query = query;
				r._query_set = true;
			}
			else
			{
				std::string	target = keyword;
				r._target = target;
				r._target_set = true;
			}
			iss>>std::ws;
			std::getline(iss, valueword, '\r');
			valueword = trim(valueword);
			if (valueword != "HTTP/1.1")
				goto reject_505;
		}
		else if (keyword == "Connection:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			std::string	lowered = toLower(valueword);
			if (lowered.empty())
				goto reject_400;
			std::istringstream	conn_stream(lowered);
			std::string			conn_token;
			while (std::getline(conn_stream, conn_token, ','))
			{
				conn_token = trim(conn_token);
				if (conn_token == "close")
					r._persistent = false;
				else if (conn_token == "keep-alive")
					continue;
			}
		}
		else if (keyword == "Host:")
		{
			if (r._host_set)
				goto reject_400;
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			if (valueword.empty())
				goto reject_400;
			std::string	host;
			std::string	port_candidate;
			bool		port_present = false;
			if (valueword[0] == '[')
				goto reject_505;
			else
			{
				std::string::size_type	colon_mark_pos = valueword.find(':');
				if (colon_mark_pos != std::string::npos)
				{
					host = valueword.substr(0, colon_mark_pos);
					port_candidate = trim(valueword.substr(colon_mark_pos + 1));
					port_present = true;
				}
				else
					host = valueword;
			}
			host = trim(host);
			if (host.empty())
				goto reject_400;
			if (port_present)
			{
				if (port_candidate.empty())
					goto reject_400;
				if (port_candidate.find_first_not_of("0123456789") != std::string::npos)
					goto reject_400;
				long	portvalue = std::strtol(port_candidate.c_str(), NULL, 10);
				if (portvalue < 0 || portvalue > 65535)
					goto reject_400;
				r._port = static_cast<int>(portvalue);
				r._port_set = true;
			}
			r._host = host;
			r._host_set = true;
		}
		else if (keyword == "Authorization:")
		{
			std::string	auth_type;
			std::string	auth_value;
			auth_type.clear();
			auth_value.clear();
			iss>>auth_type;
			iss>>std::ws;
			if (!std::getline(iss, auth_value, '\r') || auth_type.empty())
				goto reject_400;
			auth_value = trim(auth_value);
			if (auth_value.empty())
				goto reject_400;
			r._auth[auth_type] = auth_value;
			r._auth_set = true;
		}
		else if (keyword == "Cookie:")
		{
			std::string	cookie_line;
			iss>>std::ws;
			if (!std::getline(iss, cookie_line, '\r'))
				continue;
			cookie_line = trim(cookie_line);
			if (cookie_line.empty())
				continue;
			std::istringstream	cookie_stream(cookie_line);
			std::string	cookie_pair;
			while (std::getline(cookie_stream, cookie_pair, ';'))
			{
				cookie_pair = trim(cookie_pair);
				if (cookie_pair.empty())
					continue;
				std::string::size_type	equal_mark_pos = cookie_pair.find('=');
				if (equal_mark_pos == std::string::npos)
					continue;
				std::string	c1 = trim(cookie_pair.substr(0, equal_mark_pos));
				std::string	c2 = trim(cookie_pair.substr(equal_mark_pos + 1));
				if (c1.empty())
					continue;
				r._cookie[c1] = c2;
				r._cookie_set = true;
			}
			#ifdef	_DEBUG
			std::map<std::string,std::string>::const_iterator	it = r._cookie.begin();
			while (it != r._cookie.end())
			{
				std::cout<<it->first<<"\n"<<it->second<<std::endl;
				it++;
			}
			#endif
		}
		else if (keyword == "Transfer-Encoding:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r') || valueword != "chunked")
				goto reject_501;
			r._chunked = true;
		}
		else if (keyword == "Content-Length:")
			r._body_length_set = true;
		else
			continue;
	}
	if (!r._target_set || !r._host_set)
		goto reject_400;
	if (r._chunked && r._body_length_set)
		goto reject_501;
	return;

	reject_400:
	{
		r._err_code = 400;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(400);
		return;
	}
	reject_501:
	{
		r._err_code = 501;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(501);
		return;
	}
	reject_505:
	{
		r._err_code = 505;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(505);
		return;
	}
}

void	Connection::parsePOST(void)
{
	std::istringstream	iss(_header);
	std::string			line;
	std::string			keyword;
	std::string			valueword;
	bool				toggle = true;
	std::istringstream	iss2(_body);

	if (r._job_done)
		return;
	while (std::getline(iss, line))
	{
		keyword.clear();
		valueword.clear();
		if (!line.empty() && line[0] == '\r')
			return ;
		iss>>keyword;
#ifdef	_DEBUG
		std::cout<<line<<std::endl;
		std::cout<<"keyword being evaluated is:'"<<keyword<<"'"<<std::endl;
#endif
		if (toggle)
		{
			toggle = false;
			if (r._should_reject)
				return ;
			if (keyword.empty())
				goto reject_400;
			std::string::size_type	query_mark_pos = keyword.find('?');
			if (query_mark_pos != std::string::npos)
			{
				std::string	target = keyword.substr(0, query_mark_pos);
				if (query_mark_pos + 1 >= keyword.size())
				{
					r._target = target;
					r._target_set = true;
					continue;
				}
				std::string	query = keyword.substr(query_mark_pos + 1, keyword.size());
				r._target = target;
				r._target_set = true;
				r._query = query;
				r._query_set = true;
			}
			else
			{
				std::string	target = keyword;
				r._target = target;
				r._target_set = true;
			}
			iss>>std::ws;
			std::getline(iss, valueword, '\r');
			valueword = trim(valueword);
			if (valueword != "HTTP/1.1")
				goto reject_505;
		}
		else if (keyword == "Connection:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			std::string	lowered = toLower(valueword);
			if (lowered.empty())
				goto reject_400;
			std::istringstream	conn_stream(lowered);
			std::string			conn_token;
			while (std::getline(conn_stream, conn_token, ','))
			{
				conn_token = trim(conn_token);
				if (conn_token == "close")
					r._persistent = false;
				else if (conn_token == "keep-alive")
					continue;
			}
		}
		else if (keyword == "Host:")
		{
			if (r._host_set)
				goto reject_400;
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			if (valueword.empty())
				goto reject_400;
			std::string	host;
			std::string	port_candidate;
			bool		port_present = false;
			if (valueword[0] == '[')
				goto reject_505;
			else
			{
				std::string::size_type	colon_mark_pos = valueword.find(':');
				if (colon_mark_pos != std::string::npos)
				{
					host = valueword.substr(0, colon_mark_pos);
					port_candidate = trim(valueword.substr(colon_mark_pos + 1));
					port_present = true;
				}
				else
					host = valueword;
			}
			host = trim(host);
			if (host.empty())
				goto reject_400;
			if (port_present)
			{
				if (port_candidate.empty())
					goto reject_400;
				if (port_candidate.find_first_not_of("0123456789") != std::string::npos)
					goto reject_400;
				long	portvalue = std::strtol(port_candidate.c_str(), NULL, 10);
				if (portvalue < 0 || portvalue > 65535)
					goto reject_400;
				r._port = static_cast<int>(portvalue);
				r._port_set = true;
			}
			r._host = host;
			r._host_set = true;
		}
		else if (keyword == "Authorization:")
		{
			std::string	auth_type;
			std::string	auth_value;
			auth_type.clear();
			auth_value.clear();
			iss>>auth_type;
			iss>>std::ws;
			if (!std::getline(iss, auth_value, '\r') || auth_type.empty())
				goto reject_400;
			auth_value = trim(auth_value);
			if (auth_value.empty())
				goto reject_400;
			r._auth[auth_type] = auth_value;
			r._auth_set = true;
		}
		else if (keyword == "Cookie:")
		{
			std::string	cookie_line;
			iss>>std::ws;
			if (!std::getline(iss, cookie_line, '\r'))
				goto reject_400;
			cookie_line = trim(cookie_line);
			if (cookie_line.empty())
				continue;
			std::istringstream	cookie_stream(cookie_line);
			std::string	cookie_pair;
			while (std::getline(cookie_stream, cookie_pair, ';'))
			{
				cookie_pair = trim(cookie_pair);
				if (cookie_pair.empty())
					continue;
				std::string::size_type	equal_mark_pos = cookie_pair.find('=');
				if (equal_mark_pos == std::string::npos)
					continue;
				std::string	c1 = trim(cookie_pair.substr(0, equal_mark_pos));
				std::string	c2 = trim(cookie_pair.substr(equal_mark_pos + 1));
				if (c1.empty())
					continue;
				r._cookie[c1] = c2;
				r._cookie_set = true;
			}
			#ifdef	_DEBUG
			std::map<std::string,std::string>::const_iterator	it = r._cookie.begin();
			while (it != r._cookie.end())
			{
				std::cout<<it->first<<"\n"<<it->second<<std::endl;
				it++;
			}
			#endif
		}
		else if (keyword == "Transfer-Encoding:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r') || valueword != "chunked")
				goto reject_501;
			r._chunked = true;
		}
		else if (keyword == "Content-Length:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			if (valueword.empty() || valueword.find_first_not_of("0123456789") != std::string::npos)
				goto reject_400;
			long	len = std::atol(valueword.c_str());
			r._body_length = len;
			r._body_length_set = true;
		}
		else
			continue;
	}
	if (!r._target_set || !r._host_set)
			goto reject_400;
	if (r._chunked && r._body_length_set)
			goto reject_501;
/*	From here it processes the _body part */
	if (r._chunked)
	{
		std::string	body_buffer;
		std::string	chunk_line;
		bool		saw_chunk_header = false;
		while (true)
		{
			if (!std::getline(iss2, chunk_line))
			{
				if (!saw_chunk_header && body_buffer.empty() && iss2.eof())
				{
					r._body.clear();
					r._body_set = true;
					return ;
				}
					goto reject_400;
			}
			saw_chunk_header = true;
			if (!chunk_line.empty() && chunk_line[chunk_line.size() - 1] == '\r')
				chunk_line.erase(chunk_line.size() - 1);
			chunk_line = trim(chunk_line);
			if (chunk_line.empty())
				goto reject_400;
			std::string::size_type	semicolon_pos = chunk_line.find(';');
			if (semicolon_pos != std::string::npos)
				chunk_line = trim(chunk_line.substr(0, semicolon_pos));
			if (chunk_line.empty() || chunk_line.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
				goto reject_400;
			long	chunk_size = std::strtol(chunk_line.c_str(), NULL, 16);
			if (chunk_size < 0)
				goto reject_400;
			if (chunk_size == 0)
			{
				while (true)
				{
					std::string	trailer_line;
					if (!std::getline(iss2, trailer_line))
						goto reject_400;
					if (!trailer_line.empty() && trailer_line[trailer_line.size() - 1] == '\r')
						trailer_line.erase(trailer_line.size() - 1);
					if (trim(trailer_line).empty())
						break ;
				}
				break ;
			}
			std::string	chunk_buffer;
			try
			{
				chunk_buffer.resize(static_cast<std::size_t>(chunk_size));
			}
			catch (const std::exception &)
			{
				goto reject_413;
			}
			iss2.read(&chunk_buffer[0], static_cast<std::streamsize>(chunk_size));
			if (iss2.gcount() != static_cast<std::streamsize>(chunk_size))
				goto reject_400;
			body_buffer.append(chunk_buffer);
			int	cr = iss2.get();
			int	lf = iss2.get();
			if (cr == std::char_traits<char>::eof() || lf == std::char_traits<char>::eof()
				|| cr != '\r' || lf != '\n')
				goto reject_400;
		}
		r._body = _body;
		r._body_set = true;
	}
	else if (r._body_length_set)
	{
		long	length = r._body_length;
		if (length < 0)
				goto reject_400;
		std::string	body_buffer;
		if (length > 0)
		{
			try
			{
				body_buffer.resize(static_cast<std::size_t>(length));
			}
			catch (const std::exception &)
			{
				goto reject_413;
			}
			iss2.read(&body_buffer[0], static_cast<std::streamsize>(length));
			if (iss2.gcount() != static_cast<std::streamsize>(length))
				goto reject_400;
		}
		r._body = body_buffer;
		r._body_set = true;
	}
	else
	{
		r._body.clear();
		r._body_set = true;
	}
	if (!r._target_set || !r._host_set)
		goto reject_400;
	if (r._chunked && r._body_length_set)
		goto reject_501;
	return;

	reject_400:
	{
		r._err_code = 400;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(400);
		return;
	}
	reject_413:
	{
		r._err_code = 413;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(413);
		return;
	}
	reject_501:
	{
		r._err_code = 501;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(501);
		return;
	}
	reject_505:
	{
		r._err_code = 505;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(505);
		return;
	}
}

void	Connection::parseDELETE(void)
{
	std::istringstream	iss(_header);
	std::string			line;
	std::string			keyword;
	std::string			valueword;
	bool				toggle = true;
	
	if (r._job_done)
		return;
	while (std::getline(iss, line))
	{
		keyword.clear();
		valueword.clear();
		if (!line.empty() && line[0] == '\r')
			return ;
		iss>>keyword;
#ifdef	_DEBUG
		std::cout<<line<<std::endl;
		std::cout<<"keyword being evaluated is:'"<<keyword<<"'"<<std::endl;
#endif
		if (toggle)
		{
			toggle = false;
			if (keyword.empty())
				goto reject_400;
			std::string::size_type	query_mark_pos = keyword.find('?');
			if (query_mark_pos != std::string::npos)
			{
				std::string	target = keyword.substr(0, query_mark_pos);
				if (query_mark_pos + 1 >= keyword.size())
				{
					r._target = target;
					r._target_set = true;
					continue;
				}
				std::string	query = keyword.substr(query_mark_pos + 1, keyword.size());
				r._target = target;
				r._target_set = true;
				r._query = query;
				r._query_set = true;
			}
			else
			{
				std::string	target = keyword;
				r._target = target;
				r._target_set = true;
			}
			iss>>std::ws;
			std::getline(iss, valueword, '\r');
			valueword = trim(valueword);
			if (valueword != "HTTP/1.1")
				goto reject_505;
		}
		else if (keyword == "Connection:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			std::string	lowered = toLower(valueword);
			if (lowered.empty())
				goto reject_400;
			std::istringstream	conn_stream(lowered);
			std::string			conn_token;
			while (std::getline(conn_stream, conn_token, ','))
			{
				conn_token = trim(conn_token);
				if (conn_token == "close")
					r._persistent = false;
				else if (conn_token == "keep-alive")
					continue;
			}
		}
		else if (keyword == "Host:")
		{
			if (r._host_set)
				goto reject_400;
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			if (valueword.empty())
				goto reject_400;
			std::string	host;
			std::string	port_candidate;
			bool		port_present = false;
			if (valueword[0] == '[')
				goto reject_505;
			else
			{
				std::string::size_type	colon_mark_pos = valueword.find(':');
				if (colon_mark_pos != std::string::npos)
				{
					host = valueword.substr(0, colon_mark_pos);
					port_candidate = trim(valueword.substr(colon_mark_pos + 1));
					port_present = true;
				}
				else
					host = valueword;
			}
			host = trim(host);
			if (host.empty())
				goto reject_400;
			if (port_present)
			{
				if (port_candidate.empty())
					goto reject_400;
				if (port_candidate.find_first_not_of("0123456789") != std::string::npos)
					goto reject_400;
				long	portvalue = std::strtol(port_candidate.c_str(), NULL, 10);
				if (portvalue < 0 || portvalue > 65535)
					goto reject_400;
				r._port = static_cast<int>(portvalue);
				r._port_set = true;
			}
			r._host = host;
			r._host_set = true;
		}
		else if (keyword == "Authorization:")
		{
			std::string	auth_type;
			std::string	auth_value;
			auth_type.clear();
			auth_value.clear();
			iss>>auth_type;
			iss>>std::ws;
			if (!std::getline(iss, auth_value, '\r') || auth_type.empty())
				goto reject_400;
			auth_value = trim(auth_value);
			if (auth_value.empty())
				goto reject_400;
			r._auth[auth_type] = auth_value;
			r._auth_set = true;
		}
		else if (keyword == "Cookie:")
		{
			std::string	cookie_line;
			iss>>std::ws;
			if (!std::getline(iss, cookie_line, '\r'))
				continue;
			cookie_line = trim(cookie_line);
			if (cookie_line.empty())
				continue;
			std::istringstream	cookie_stream(cookie_line);
			std::string	cookie_pair;
			while (std::getline(cookie_stream, cookie_pair, ';'))
			{
				cookie_pair = trim(cookie_pair);
				if (cookie_pair.empty())
					continue;
				std::string::size_type	equal_mark_pos = cookie_pair.find('=');
				if (equal_mark_pos == std::string::npos)
					continue;
				std::string	c1 = trim(cookie_pair.substr(0, equal_mark_pos));
				std::string	c2 = trim(cookie_pair.substr(equal_mark_pos + 1));
				if (c1.empty())
					continue;
				r._cookie[c1] = c2;
				r._cookie_set = true;
			}
			#ifdef	_DEBUG
			std::map<std::string,std::string>::const_iterator	it = r._cookie.begin();
			while (it != r._cookie.end())
			{
				std::cout<<it->first<<"\n"<<it->second<<std::endl;
				it++;
			}
			#endif
		}
		else if (keyword == "Transfer-Encoding:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r') || valueword != "chunked")
				goto reject_501;
			r._chunked = true;
		}
		else if (keyword == "Content-Length:")
			r._body_length_set = true;
		else
			continue;
	}
	if (!r._target_set || !r._host_set)
		goto reject_400;
	if (r._chunked && r._body_length_set)
		goto reject_501;
	if (!r._cookie_set)
		goto reject_403;
	return;

	reject_400:
	{
		r._err_code = 400;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(400);
		return;
	}
	reject_403:
	{
		r._err_code = 403;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(403);
		return;
	}
	reject_501:
	{
		r._err_code = 501;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(501);
		return;
	}
	reject_505:
	{
		r._err_code = 505;
		r._err_code_set = true;
		r._should_reject = true;
		sendErrPage(505);
		return;
	}
}

void	Connection::disengageLoop(void)
{
	if (_engaged && _loop)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::queueWrite(const std::string &data)
{
	int	events = EVENT_READ;
	
	if (_fd < 0)
		return ;
	_outbuf.append(data);
	if (_loop && _engaged)
	{
		if (!_outbuf.empty())
			events = events | EVENT_WRITE;
		_loop->set_events(_fd, events);
	}
}

void	Connection::takeInput(std::string &dest)
{
	dest.swap(_inbuf);
}

void	Connection::requestClose(void)
{
	_should_close = true;
}

int	Connection::getFD(void) const
{
	return (_fd);
}

const std::string	&Connection::getServerName(void) const
{
	return (_server_name);
}

bool	Connection::isEngaged(void) const
{
	return (_engaged);
}

bool	Connection::isClose(void) const
{
	return (_fd < 0);
}

void	Connection::onReadable(int fd)
{
	char	buf[8192];
	ssize_t	n = ::recv(_fd, buf, static_cast<int>(8192u), 0);
	(void)fd;
	if (n > 0)
	{
		_inbuf.append(buf, static_cast<size_t>(n));
		requestProccess();
	}
	else if (!n)
		_should_close = true;
	/* n < 0 must be ignored, otherwise this shit blocks and all fuck up*/
}

void	Connection::resetRequest(void)
{
	r._target.clear();
	r._target_set = false;
	r._query.clear();
	r._query_set = false;
	r._host.clear();
	r._host_set = false;
	r._port = 0;
	r._port_set = false;
	r._auth.clear();
	r._auth_set = false;
	r._cookie.clear();
	r._cookie_set = false;
	r._should_reject = false;
	r._persistent = true;
	r._chunked = false;
	r._body_length = 0;
	r._body_length_set = false;
	r._err_code = 0;
	r._err_code_set = false;
	r._body.clear();
	r._body_set = false;
	r._job_done = false;
	_should_close = false;
	_header.clear();
	_body.clear();
}

void	Connection::requestProccess(void)
{
	Connection::resetRequest();
#ifdef	_DEBUG
	std::cout<<"\n---received request---\n"<<_inbuf<<"---end of request---\n"<<std::endl;
#endif
	std::string::size_type	header_end = _inbuf.find("\r\n\r\n");
	if (r._job_done || _inbuf.size() < 16 || header_end == std::string::npos)
	{
		sendErrPage(400);
		return ;
	}
	_header = _inbuf.substr(0, header_end);
	if (header_end + 4 < _inbuf.size())
		_inbuf.erase(0, header_end + 4);
	else
		_inbuf.clear();
	_body = _inbuf;
#ifdef	_DEBUG
	std::cout<<"\n---body content (if any) ---"<<_body<<"---end of body---"<<std::endl;
#endif
	std::istringstream	iss(_header);
	std::string	word;
	iss>>word;
	if (word == "GET")
	{
		_method = GET_MASK;
		parseGET();
	}
	else if (word == "POST")
	{
		_method = POST_MASK;
		parsePOST();
	}
	else if (word == "DELETE")
	{
		_method = DELETE_MASK;
		parseDELETE();
	}
	else if (word == "PATCH" || word == "OPTIONS" || word == "CONNECT"
			|| word == "PUT" || word == "TRACE" || word == "HEAD")
			sendErrPage(405);
	else
		sendErrPage(400);
}

void	Connection::onWritable(int fd)
{
	ssize_t	n;
	int		events = EVENT_READ;
	(void)fd;

	if (!_outbuf.empty())
	{
		n = ::send(_fd, _outbuf.data(), static_cast<int>(_outbuf.size()), 0);
		if (n > 0)
			_outbuf.erase(0, static_cast<size_t>(n));
	}
	if (_loop && _engaged)
	{
		if (!_outbuf.empty())
			events = events | EVENT_WRITE;
		_loop->set_events(_fd, events);
	}
	if (_should_close && _outbuf.empty())
	{
		if (_loop && _engaged)
		{
			_loop->remove(_fd);
			_engaged = false;
		}
		if (_fd > -1)
		{
			(void)::close(_fd);
			_fd = -1;
		}
	}
}

void	Connection::onError(int fd)
{
	(void)fd;
	
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::onHangup(int fd)
{
	(void)fd;
	
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::onTick(int fd)
{
	(void)fd;
	
	if (_should_close && _outbuf.empty())
	{
		if (_loop && _engaged)
		{
			_loop->remove(_fd);
			_engaged = false;
		}
		if (_fd > -1)
		{
			(void)::close(_fd);
			_fd = -1;
		}
	}
}
