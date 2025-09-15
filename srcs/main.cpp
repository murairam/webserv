/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:13:45 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 16:34:25 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "_headers.hpp"
# include "ConfigLoader.hpp"
# include "SysError.hpp"

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
		{
			ConfigLoader	cfg;
		}
		else
			ConfigLoader	cfg(argv[1]);
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
