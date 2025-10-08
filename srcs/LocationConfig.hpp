/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:14:10 by yanli             #+#    #+#             */
/*   Updated: 2025/10/01 13:11:53 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "_headers.hpp"

class	LocationConfig
{
	private:
		std::string		_path_prefix;
		int				_allowed_methods;
		std::vector<std::string>	_index_files;
		bool			_autoindex;
		std::string		_alias;
		int				_redirect_code;
		std::string		_redirect_target;
		long			_client_max_body_size_override;
		int				_priority;
		std::map<std::string, std::string>	_cgi_handlers;

	public:
		LocationConfig(void);
		LocationConfig(const LocationConfig &other);
		LocationConfig	&operator=(const LocationConfig &other);
		~LocationConfig(void);

		const std::string	&getPathPrefix(void) const;
		int					getAllowedMethods(void) const;
		bool				MethodIsAllowed(int method_mask) const;
		bool				getAutoindex(void) const;
		bool				getUploadEnabled(void) const;
		const std::string	&getAlias(void) const;
		std::string			getCgi(std::string ext) const;
		int					getRedirectCode(void) const;
		const std::string	&getRedirectTarget(void) const;
		long				getClientBodyLimit(void) const;
		int					getPriority(void) const;
		const std::vector<std::string>	&getIndexFiles(void) const;

		void	setPathPrefix(const std::string &prefix);
		void	setMethod(int method_mask);
		void	addIndexFile(std::string file);
		void	setAutoindex(bool enabled);
		void	setAlias(const std::string &path);
		void	setRedirect(int code, const std::string &target);
		void	setClientBodyLimit(long limit);
		void	setPriority(int priority);
		void	addCgiHandler(const std::string &ext, const std::string &program);
		const std::map<std::string, std::string> &getCgiHandlers(void) const;

#ifdef	_DEBUG
		void	debug(void) const;
#endif
};

#endif
