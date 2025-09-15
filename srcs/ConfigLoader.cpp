/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 20:40:31 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 14:55:54 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"

// Helper: Set up CGI executables for a LocationConfig (for testing/demo)
void setupDefaultCgiExecutables(LocationConfig &loc) {
	loc.setCgiExecutable(".php", "/usr/bin/php-cgi");
	loc.setCgiExecutable(".py", "/usr/bin/python3");
	loc.setCgiExecutable(".rb", "/usr/bin/ruby");
}

// Test block: Demonstrate CGI mapping works
#include <iostream>
void testCgiMapping() {
	LocationConfig loc;
	setupDefaultCgiExecutables(loc);
	std::cout << "PHP CGI: " << loc.getCgi(".php") << std::endl;
	std::cout << "Python CGI: " << loc.getCgi(".py") << std::endl;
	std::cout << "Ruby CGI: " << loc.getCgi(".rb") << std::endl;
	std::cout << "Unknown CGI: '" << loc.getCgi(".pl") << "' (should be empty)" << std::endl;
}
// Example usage (remove or adapt for your actual config loading logic):
// LocationConfig loc;
// setupDefaultCgiExecutables(loc);
// ...
