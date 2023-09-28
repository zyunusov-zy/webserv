#ifndef SERVER_HPP
# define SERVER_HPP

#include "mainIn.hpp"
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

#include "Client.hpp"
#include "mainIn.hpp"
#include "Request.hpp"

class Server
{
private:
	// Config _conf;
	std::string readFile(const std::string & filename);
	std::vector<t_serv> servers;


public:
	Server();
	~Server();

	std::map<int, Client> fd_to_clients;
	std::map<int, Client>::iterator client_it;
	// void conf(std::string filename,std::vector<t_serv>& servers);
	void setUp(std::vector<t_serv>& s);
	void launchCgi(Client client, int fd);
    void sendHTMLResponse(Client client, int fd, std::string filepath);
	//-----
	
	//-----
};
#endif
