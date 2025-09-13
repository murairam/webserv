/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 18:18:55 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 18:45:43 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GLOBALCONFIG_HPP
# define GLOBALCONFIG_HPP

# include <string>
# include <map>

class	GlobalConfig
{
	private:
		std::string		_config_path;
		int				_poll_timeout_ms;
		int				_idle_connection_ms;
		int				_max_header_bytes;
		int				_max_request_line_bytes;
		std::string		_default_mime;
		std::map<std::string, std::string>	_mime_overrides;
		std::string		_default_error_pages_root;
		std::string		_default_server_name;

	public:	
		GlobalConfig(void);
		GlobalConfig(const GlobalConfig &other);
		GlobalConfig	&operator=(const GlobalConfig &other);
		~GlobalConfig(void);
		
		/* Getters */
		const std::string	&getConfigPath(void) const;
		int	getPollTimeoutMs(void) const;
		int	getIdleConnectionMs(void) const;
		int	getMaxHeaderBytes(void) const;
		int	getMaxRequestLineBytes(void) const;
		const std::string	&getDefaultMime(void) const;
		const std::map<std::string,std::string>	&getMimeOverrides(void) const;
		const std::string	&getDefaultErrorPagesRoot(void) const;
		const std::string	&getHardDefaultServerName(void) const;

		/* Setters */
		void	setConfigPath(const std::string &path);
		void	setPollTimeoutMs(int n);
		void	setIdleConnectionMs(int n);
		void	setMaxHeaderBytes(int n);
		void	setMaxRequestLineBytes(int n);
		void	setDefaultMime(const std::string &mime);
		void	setMimeOverrides(const std::string &ext, const std::string &mime);
		void	setDefaultErrorPagesRoot(const std::string &path);
};

#endif
