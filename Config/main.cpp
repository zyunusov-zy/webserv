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
		try{
			_conf.parse(av[1], servers);
		}catch(ErrorException &e)
		{
			std::cerr << e.what() << std::endl;
			return (1);
		}

		std::cout << "SIZE_OF_VECTOR: " << servers.size() << std::endl;
		// std::cout << "EEEEE" << servers[1].host << std::endl;
		// serv.conf(av[1], servers);
		Server serv;

		// exit(1);

		serv.setUp(servers);
	}
	else
	{
		std::cerr << RED << "Invalid number of arguments." << NORMAL << std::endl;
		return (1);
	}
}