/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:13:45 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 14:23:57 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "_headers.hpp"
# include "ConfigLoader.hpp"
# include "SysError.hpp"
# include "Header.hpp"
# include "debug.hpp"


int	main(int argc, char **argv, char **envp)
{
	try
	{
		if (!envp || !*envp || !**envp)
		{
			std::cerr<<ERROR_MSG_INVALID_ENVP<<std::endl;
			return (1);
		}
		if (argc == 1)
			ConfigLoader	cfg;
		else
		{
			ConfigLoader	cfg(argv[1]);
#ifdef	_DEBUG
			cfg.debug();
#endif
			if (cfg.selfcheck())
				return (2);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr<<e.what()<<std::endl;
	}
	catch (...)
	{
		std::cerr<<"Non-standard exception caught"<<std::endl;
	}
	return (0);
}

/* this one is to test Get/Post/Delete Request parsing */
/*
int	main(int argc, char **argv, char **envp)
{
	try
	{
		(void)argc;
		(void)argv;
		(void)envp;
		std::istringstream	input(GET_REQUEST);
		std::istringstream	iss(GET_REQUEST);
		std::string	str(iss.str());
		std::cout<<str<<std::endl;
		Header	test(input);
		if (test.shouldReject())
			return (1);
	}
	catch (const std::exception &e)
	{
		std::cerr<<e.what()<<std::endl;
	}
	catch (...)
	{
		std::cerr<<"Non-standard exception caught"<<std::endl;
	}
	return (0);
}*/
/* this one is to test Response class
void testResponse()
{
    std::cout << "\n=== TESTING RESPONSE CLASS ===" << std::endl;

    try {
        // Test basic response
        std::cout << "1. Testing basic response..." << std::endl;
        Response resp(200);
        resp.setBody("Hello World!");
        std::cout << resp.serialize() << std::endl;

        // Test error response
        std::cout << "\n2. Testing error response..." << std::endl;
        Response error_resp = Response::createErrorResponse(404);
        std::string error_output = error_resp.serialize();
        std::cout << "Status line: " << error_output.substr(0, error_output.find('\r')) << std::endl;
        std::cout << "Body length: " << error_output.substr(error_output.find("\r\n\r\n") + 4).length() << std::endl;

        // Test file response
        std::cout << "\n3. Testing file response..." << std::endl;
        Response file_resp = Response::createFileResponse("./assets/error_pages/404.html");
        std::string file_output = file_resp.serialize();
        std::cout << "Status: " << (file_output.find("200 OK") != std::string::npos ? "✅ PASS" : "❌ FAIL") << std::endl;
        std::cout << "Content-Type: " << (file_output.find("text/html") != std::string::npos ? "✅ PASS" : "❌ FAIL") << std::endl;

    } catch (const std::exception &e) {
        std::cout << "Test exception: " << e.what() << std::endl;
    }
}

int main(int argc, char **argv, char **envp)
{
    try
    {
        if (!envp || !*envp || !**envp)
        {
            std::cerr << ERROR_MSG_INVALID_ENVP << std::endl;
            return (1);
        }

        // Test Response class
        testResponse();

        // Your existing config code...
        if (argc == 1) {
            ConfigLoader cfg;
        } else {
            ConfigLoader cfg(argv[1]);
#ifdef _DEBUG
            cfg.debug();
#endif
            if (cfg.selfcheck())
                return (2);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return (0);
}*/
