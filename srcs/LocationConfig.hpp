/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:14:10 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 14:49:04 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "_headers.hpp"


class LocationConfig {
private:
	std::string _path_prefix;
	int _allowed_methods;
	std::string _root;
	std::vector<std::string> _index_files;
	bool _autoindex;
	bool _upload_enabled;
	std::string _upload_path;
	int _redirect_code;
	std::string _redirect_target;
	long _client_max_body_size_override;
	int _priority;
	std::map<std::string, std::string> _cgiExecutables; // extension -> executable

public:
	LocationConfig(void);
	LocationConfig(const LocationConfig &other);
	LocationConfig &operator=(const LocationConfig &other);
	~LocationConfig(void);

	const std::string &getPathPrefix(void) const;
	int getAllowedMethods(void) const;
	const std::string &getRoot(void) const;
	bool getAutoindex(void) const;
	bool getUploadEnabled(void) const;
	const std::string &getUploadPath(void) const;
	std::string getCgi(const std::string &ext) const;
	int getRedirectCode(void) const;
	const std::string &getRedirectTarget(void) const;
	long getClientBodyLimit(void) const;
	int getPriority(void) const;
	const std::vector<std::string> &getIndexFiles(void) const;

	// New for bonus: manage CGI executables
	void setCgiExecutable(const std::string &ext, const std::string &exec);
	const std::map<std::string, std::string> &getCgiExecutables() const;
};

#endif
