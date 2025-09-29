/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:12:09 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/09/29 17:21:55 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include "_headers.hpp"

class HttpRequest
{
private:
    // Note: Making members private per 42 norm
    std::string                         _method;
    std::string                         _path;
    std::string                         _query;
    std::string                         _version;
    std::map<std::string, std::string>  _headers;
    std::map<std::string, std::string>  _cookies;
    std::string                         _body;
    bool                                _persistent;
    bool                                _chunked;
    long                                _content_length;
    bool                                _should_reject;
    int                                 _error_code;
	bool								_header_set;

public:
    // Orthodox Canonical Form
    HttpRequest(void);
    HttpRequest(const HttpRequest &other);
    HttpRequest &operator=(const HttpRequest &other);
    ~HttpRequest(void);

    // Getters 
    const std::string   &getMethod(void) const;
    const std::string   &getPath(void) const;
    const std::string   &getQuery(void) const;
    const std::string   &getVersion(void) const;
    const std::string   &getBody(void) const;
    bool                getPersistent(void) const;
    bool                getChunked(void) const;
    long                getContentLength(void) const;
    bool                getShouldReject(void) const;
    int                 getErrorCode(void) const;

    // Setters
    void    setMethod(std::string method);
    void    setPath(std::string path);
    void    setQuery(std::string query);
    void    setVersion(std::string version);
    void    setBody(std::string body);
    void    setPersistent(bool persistent);
    void    setChunked(bool chunked);
    void    setContentLength(long length);
    void    setShouldReject(bool reject);
    void    setErrorCode(int code);

    // Header and cookie management
    bool        addHeader(std::string key, std::string value);
    void        addCookie(std::string key, std::string value);
    std::string getHeader(const std::string &name) const;
    std::string getCookie(const std::string &name) const;
    bool        hasHeader(const std::string &name) const;

    // Future CGI support
    std::map<std::string, std::string> toCgiEnvironment(void) const;

#ifdef _DEBUG
    void    debug(void) const;
#endif
};

#endif
