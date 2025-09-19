/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 01:25:04 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 20:44:52 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "_headers.hpp"
# include "CodePage.hpp"

class	Response
{
	private:
		int									_status_code;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		CodePage							_code_page;

		std::string	intToString(size_t value) const;
		std::string	getFileExtension(const std::string &filepath) const;
		std::string	getMimeType(const std::string &extension) const;
		std::string	generateDirectoryListingHTML(const std::string &path, const std::string &uri) const;

	public:
		Response(void);
		Response(int status_code);
		Response(const Response &other);
		Response	&operator=(const Response &other);
		~Response(void);

		void		setStatus(int code);
		void		setHeader(const std::string &key, const std::string &value);
		void		setBody(const std::string &body);
		void		setBodyFromFile(const std::string &filepath);
		std::string	serialize(void) const;

#ifdef _DEBUG
		void		debug(void) const;
#endif

		static Response	createErrorResponse(int code, const std::string &error_page_path = "");
		static Response	createFileResponse(const std::string &filepath);
		static Response	createRedirectResponse(int code, const std::string &location);
		static Response	createDirectoryListing(const std::string &path, const std::string &uri);
};

#endif
