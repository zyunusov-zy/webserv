#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include "Config.hpp"

# define RED "\033[31m"
# define NORMAL "\033[0m"

class Server
{
private:
	Config  _conf;
public:
	Server(/* args */);
	~Server();

	void conf(std::string filename);
};

Server::Server()
{
}

void Server::conf(std::string filename)
{
	_conf.parse(filename);
}

Server::~Server()
{
}


#endif