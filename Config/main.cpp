#include "Server.hpp"
#include "Config.hpp"
#include <unistd.h>
#include <cstdlib>
#include <iostream>

# define RED "\033[31m"
# define NORMAL "\033[0m"

int main(int argc, char** av)
{
    Config _conf;
    std::vector<t_serv> servers;
    const char* configFile;

    // Check arguments
    if (argc == 2)
    {
        configFile = av[1];
    }
    else if (argc == 1)
    {
        configFile = "/home/zyko/webserv/Config/t.conf";
    }
    else
    {
        std::cerr << RED << "Invalid number of arguments." << NORMAL << std::endl;
        return (1);
    }
    // Parse config
    try
    {
        _conf.parse(configFile, servers);
    }
    catch(ErrorException &e)
    {
		std::cerr << "Here tho" << std::endl;
        std::cerr << e.what() << std::endl;
        return (1);
    }

    Server serv;
    serv.setUp(servers);

    return 0; // Assuming success if no exceptions were thrown
}
