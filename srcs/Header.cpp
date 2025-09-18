/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:24:01 by yanli             #+#    #+#             */
/*   Updated: 2025/09/18 19:59:03 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Header.hpp"

Header::Header(void)
:_request_method(0), _get(), _post(), _delete(),
_should_reject(false), _reject_405(false)
{}

Header::Header(const Header &other)
:_request_method(other._request_method), _get(other._get),
_post(other._post), _delete(other._delete),
_should_reject(other._should_reject), _reject_405(other._reject_405)
{}

Header	&Header::operator=(const Header &other)
{
	if (this != &other)
	{
		_request_method = other._request_method;
		_get = other._get;
		_post = other._post;
		_delete = other._delete;
		_should_reject = other._should_reject;
		_reject_405 = other._reject_405;
	}
	return (*this);
}

Header::~Header(void) {}

Header::Header(std::istream &s)
:_request_method(0), _get(), _post(), _delete(),
_should_reject(false), _reject_405(false)
{
	if (process(s))
		_should_reject = true;
}

int	Header::process(std::istream &s)
{
	std::string			method_word;

	method_word.clear();
	if (!(s>>method_word))
		return (400);
	if (method_word == "GET")
	{
		_request_method = GET_MASK;
		_get = GetRequest(s);
		return (_get.shouldReject());
	}
	else if (method_word == "POST")
	{
		_request_method = POST_MASK;
		_post = PostRequest(s);
#ifdef	_DEBUG
		std::cout<<"PostRequest debug info of the body part:\n"<<_post.getBody()<<std::endl;
#endif
		return (_post.shouldReject());
	}
	else if (method_word == "DELETE")
	{
		_request_method = DELETE_MASK;
		_delete = DeleteRequest(s);
		return (_delete.shouldReject());
	}
	return (405);
}

int	Header::rejectCode(void) const
{
	if (_reject_400)
		return (400);
	if (_reject_405)
		return (405);
	if (_request_method == GET_MASK && _get.shouldReject())
		return (_get.getErrCode());
	else if (_request_method == POST_MASK && _post.shouldReject())
		return (_post.getErrCode());
	else if (_request_method == DELETE_MASK && _delete.shouldReject())
		return (_delete.getErrCode());
	return (400);
}

bool	Header::shouldReject(void) const
{
	return (_should_reject);
}
