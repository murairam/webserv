/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 14:17:33 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 15:10:52 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONMANAGR_HPP
# define CONNECTIONMANAGER_HPP

# include "_headers.hpp"

class	EventLoop;
class	Connection;

/*	ConnectionManager carries all Connection objects
	It creates new connections from accepted FDs;
	It updates the record fd<---->Connection*;
*/
class	ConnectionManager
{
	private:
		std::map<int,Connection*>	_conns;

		ConnectionManager(const ConnectionManager &other);
		ConnectionManager	&operator=(const ConnectionManager &other);
	
	public:
		ConnectionManager(void);
		~ConnectionManager(void);
		
		Connection	*establish(int client_fd, const std::string &server_name, EventLoop &loop);
		void	drop(int fd);
		void	drop_all(void);
		Connection	*getConn(int fd) const;
		size_t	getMapSize(void) const;
};

#endif
