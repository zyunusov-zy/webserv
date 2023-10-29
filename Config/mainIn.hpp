#ifndef MAININ_HPP
#define MAININ_HPP

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>
#include <poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

// #include "Server.hpp"
// #include "Request.hpp"
// #include "Client.hpp"
// #include "Response.hpp"
#include "ErrorCodes.hpp"
#include "Location.hpp"
// #include "Config.hpp"


typedef struct s_serv
{
	std::string								name;
	std::string 							host;
	std::vector<int>						port;
	std::string								Mroot;
	std::map<int, std::string>				errorPages;
	std::multimap<std::string, Location>			loc;
	std::vector<std::string>				ipPort;
}	t_serv;

#endif