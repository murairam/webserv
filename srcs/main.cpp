/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:13:45 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 12:58:16 by mmiilpal         ###   ########.fr       */
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
