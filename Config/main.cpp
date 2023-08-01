#include "Server.hpp"

int main(int argc, char** av)
{
	if (argc == 2)
	{
		Server serv;
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