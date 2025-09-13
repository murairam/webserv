/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:14:10 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 14:46:33 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include <string>
# include <vector>
# include <stdexcept>
# include "CgiMapping.hpp"

class	LocationConfig
{
	private:
		std::string		_path_prefix;
		int				_allowed_methods;
		std::string		_root;
		std::vector<std::string>	_index_files;
		bool			_autoindex;
		bool			_upload_enabled;
		std::string		_upload_path;
		CgiMapping		_cgi;
		int				_redirect_code;
		std::string		_redirect_target;
		long			_client_max_body_size_override;
		int				_priority;

	public:
		LocationConfig(void);
		LocationConfig(const LocationConfig &other);
		LocationConfig	&operator=(const LocationConfig &other);
		~LocationConfig(void);

		const std::string	&getPathPrefix(void) const;
		int					getAllowedMethods(void) const;
		const std::string	&getRoot(void) const;

		const std::vector<std::string>	&getIndexFiles(void) const;

		bool				getAutoindex(void) const;
		bool				getUploadEnabled(void) const;
		const std::string	&getUploadPath(void) const;
		const CgiMapping	&getCgi(void) const;
		int					getRedirectCode(void) const;
		const std::string	&getRedirectTarget(void) const;
		long				getClientMax(void) const;
		int					getPriority(void) const;
};

#endif
