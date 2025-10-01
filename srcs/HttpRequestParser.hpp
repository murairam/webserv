/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                        j        +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:31:37 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/09/29 19:56:31 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSER_HPP
# define HTTPREQUESTPARSER_HPP

# include "_headers.hpp"
# include "HttpRequest.hpp"

/*
    HttpRequestParser is a static utility class that parses HTTP requests.
    It replaces the duplicate parsing logic in Connection::parseGET/POST/DELETE
    with a single, unified parser that handles all HTTP methods.

    Usage:
        HttpRequest request = HttpRequestParser::parse(input_stream);
        if (request.getShouldReject()) {
            // Handle error using request.getErrorCode()
        }
*/
class HttpRequestParser
{
private:
    // Private constructor - this is a static utility class
    HttpRequestParser(void);
    HttpRequestParser(const HttpRequestParser &other);
    HttpRequestParser &operator=(const HttpRequestParser &other);
    ~HttpRequestParser(void);

    // Internal parsing helper methods
    static bool parseRequestLine(std::istream &input, HttpRequest &request);
    static bool parseHeaders(std::istream &input, HttpRequest &request);
    static bool parseBody(std::istream &input, HttpRequest &request);
    static bool parseChunkedBody(std::istream &input, HttpRequest &request);
    static bool parseFixedLengthBody(std::istream &input, HttpRequest &request);

    // Header-specific parsing helpers
    static void	parseHostHeader(const std::string &value, HttpRequest &request);
    static void	parseConnectionHeader(const std::string &value, HttpRequest &request);
    static void	parseCookieHeader(const std::string &value, HttpRequest &request);
    static void	parseContentLengthHeader(const std::string &value, HttpRequest &request);
    static void	parseTransferEncodingHeader(const std::string &value, HttpRequest &request);

	// Utility methods
	static std::string	extractHeaderName(const std::string &line);
	static std::string	extractHeaderValue(const std::string &line);
	static bool	isValidMethod(const std::string &method);
	static bool	isValidHttpVersion(const std::string &version);
	static void	setError(HttpRequest &request, int error_code);

	// URL parsing utilities
	static bool	parseUrl(const std::string &url, std::string &path, std::string &query);
	static bool	urlDecode(const std::string &encoded, std::string &decoded);
	static bool	sanitizePath(const std::string &decoded_path, std::string &sanitized);

public:
	// Main parsing interface
	static HttpRequest parse(std::istream &input);
	static HttpRequest parse(const std::string &input);

	// Validation utilities (public for testing/debugging)
	static bool	isCompleteRequest(const std::string &input);
	static bool	hasValidRequestLine(const std::string &input);
};

#endif
