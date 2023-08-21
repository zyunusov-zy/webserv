#include "Server.hpp"
#include <unistd.h>
#include <cstdlib>
#include <iostream>



# define RED "\033[31m"
# define NORMAL "\033[0m"
 


int main(int argc, char** av)
{
	if (argc == 2)
	{
		class Server serv;
		serv.conf(av[1]);
		serv.setUp();
		exit(1);
	}
	else
	{
		std::cerr << RED << "Invalid number of arguments." << NORMAL << std::endl;
		return (1);
	}
}