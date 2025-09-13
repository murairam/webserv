/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerTable.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 17:34:46 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 22:35:05 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERTABLE_HPP
# define SERVERTABLE_HPP

# include <map>
# include <vector>
# include <string>
# include <cctype>
# include <sstream>
# include <algorithm>
# include <utility>
# include "Endpoint.hpp"
# include "ServerConfig.hpp"

class	ServerTable
{
	private:
		/* From "host:port" to find which server listens to this */
		std::map<std::string, std::vector<int> >	_endpoint;
		std::vector<ServerConfig>					_servers;
		/* From server_name to find which server to use */
		std::map<std::string, int>					_name_index;

		static std::string	makeEndpointPair(const std::string &host, int port);
		/* Tells you if an index is valid */
		static bool			checkIndex(const std::vector<int> &v, int idx);
		void			buildEndpointIndex(void);
		/* Make all letters lowercase */
		static std::string	sanitizeHostHeader(const std::string &input);
	
	public:
		ServerTable(void);
		ServerTable(const ServerTable &other);
		ServerTable	&operator=(const ServerTable &other);
		~ServerTable(void);

		/* Return (-1) if duplicated name, returns the index if valid */
		int	addServer(const ServerConfig &srv);
		/* After all servers have been added, call this once to compute Endpoint map */
		bool	finalize(void);
		/* Return a list of unique endpoint pairs "host:port" */
		std::vector<std::string>	getListenerPlan(void) const;
		/* From the endpoint pair "host:port" and the host header, try to find a suitable server */
		bool	selectServer(const std::string &endpointpair, const std::string &hostheader, int &outindex) const;
		/* Getters */
		const std::vector<ServerConfig>	&getServers(void) const;
		const std::map<std::string, std::vector<int> >	&getEndpointIndex(void) const;
		const std::map<std::string, int>	&getServerIndex(void) const;
};

#endif
