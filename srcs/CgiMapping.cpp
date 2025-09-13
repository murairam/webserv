/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiMapping.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:03:19 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 14:11:43 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiMapping.hpp"

CgiMapping::CgiMapping(void)
{}

CgiMapping::CgiMapping(const CgiMapping &other)
:_data(other._data) {}

CgiMapping	&CgiMapping::operator=(const CgiMapping &other)
{
	if (this != &other)
		_data = other._data;
	return (*this);
}

CgiMapping::~CgiMapping(void) {}

const std::string	&CgiMapping::getCgiProcessor(std::string name) const
{
	std::map<std::string, std::string>::const_iterator	it = _data.find(name);
	if (it != _data.end())
		return (it->second);
	throw std::runtime_error("No suitable CGI processor for the given file");
}
