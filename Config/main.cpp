#include "Server.hpp"
#include "Config.hpp"
#include <unistd.h>
#include <cstdlib>
#include <iostream>



# define RED "\033[31m"
# define NORMAL "\033[0m"
 


int main(int argc, char** av)
{
	if (argc == 2)
	{
		Config _conf;
		std::vector<t_serv> servers;
		_conf.parse(av[1], servers);
		// std::cout << "EEEEE" << servers[1].host << std::endl;
		// serv.conf(av[1], servers);
		Server serv;

		exit(1);

		// serv.setUp(servers);
	}
	else
	{
		std::cerr << RED << "Invalid number of arguments." << NORMAL << std::endl;
		return (1);
	}
}