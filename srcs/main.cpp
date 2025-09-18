/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:13:45 by yanli             #+#    #+#             */
/*   Updated: 2025/09/18 11:38:46 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "_headers.hpp"
#include "Socket.hpp"
#include <cstring>
#define TEST_CGI_MAPPING
#ifdef TEST_CGI_MAPPING
// Forward declaration from ConfigLoader.cpp
void testCgiMapping();
#endif


int main(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;
#ifdef TEST_CGI_MAPPING
	testCgiMapping();
	return 0;
#endif
	if (!envp || !*envp || !**envp)
	{
		std::cerr << ERROR_MSG_INVALID_ENVP << std::endl;
		return 1;
	}

	try {
		// Listen on 127.0.0.1:8080
		Socket server(AF_INET, SOCK_STREAM, 0);
		server.setReuseAddr(true);
		server.setNonBlocking(false);

		struct sockaddr_in addr;
		std::memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(8080);
		server.bindTo(reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		server.listenOn(10);
		std::cout << "Listening on port 8080..." << std::endl;

		struct sockaddr_storage client_addr;
		socklen_t client_len = sizeof(client_addr);
		Socket client = server.acceptOne(client_addr, client_len);
		std::cout << "Accepted a connection!" << std::endl;

		const char *msg = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
		client.sendIO(msg, std::strlen(msg), 0);
		client.closeFD();
		server.closeFD();
	} catch (const SysError &e) {
		std::cerr << "SysError: " << e.what() << std::endl;
		return 1;
	} catch (const std::exception &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
