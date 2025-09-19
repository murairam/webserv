/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CodePage.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 17:49:32 by yanli             #+#    #+#             */
/*   Updated: 2025/09/18 19:06:48 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEPAGE_HPP
# define CODEPAGE_HPP

# include "_headers.hpp"

/*
	CodePage automatically generate apprioriate html pages for responding
	This class is not supposed to be copyable.
	
	_codes:	map of (HTTP status code -> reason phrase).
	_template_prefix: fixed HTML prefix before dynamic parts.
	_template_suffix: fixed HTML suffix after dynamic parts.
	_note_prefix: small fixed text shown before code+reason line.

	Only GET/POST/DELETE are expected. no_body_response(method) returns
	false for these; it would return true for "HEAD" if ever used.
*/

class	CodePage
{
	private:
		static const std::map<int,std::string>	&codes(void);
		const std::string			_template_prefix;
		const std::string			_template_middle;
		const std::string			_template_suffix;
		
		std::string	buildPage(int code) const;
		static std::string	to_upper(const std::string &s);
		static std::string	int_to_string(int code);
/*		static bool			no_body_response(const std::string &method);*/

		CodePage(const CodePage &other);
		CodePage	&operator=(const CodePage &other);
		
	public:
		CodePage(void);
		~CodePage(void);

		std::string	getCodePage(int code) const;
/*		std::string	getCodePage(int code, std::string s) const;*/
		std::string	getReason(int code) const;
};

#endif
