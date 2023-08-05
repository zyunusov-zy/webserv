#ifndef SERVER_HPP
# define SERVER_HPP

# define RED "\033[31m"
# define NORMAL "\033[0m"

#include <iostream>
// to set up a server
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include "Request.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "ErrorCodes.hpp"



class Server
{
private:
	Config  _conf;

	std::string readFile(const std::string & filename);
public:
	Server(/* args */);
	~Server();

	void conf(std::string filename);
	void setUp();
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

std::string Server::readFile(const std::string & filename)
{
	std::ifstream file(filename.c_str());
	std::stringstream buf;
	buf << file.rdbuf();
	return buf.str();
}

void Server::setUp()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0)
	{
		std::cerr << "Failed to create socket! \n";
		exit(1);
	}

	struct sockaddr_in address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;// here needs to be IP address
	// if (inet_pton(AF_INET, "192.168.1.100", &address.sin_addr)<=0) {
    //     std::cerr << "Invalid address or address not supported\n";
    //     return -1;
    // }
	address.sin_port = htons(_conf.servers[0].port);

	int b = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	if (b < 0)
	{
		std::cerr << "Failed to bind! \n";
		exit(1);
	}

	if (listen(server_fd, 3) < 0)
	{
		std::cerr << "Failed to listen! \n";
		exit(1);
	}

	// std::ofstream file("request.txt", std::ios::binary | std::ios::out);

	while(true)
	{
		std::cout << "Waiting for a connection...\n";
		socklen_t addrlen = sizeof(address);
		int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
		if (new_socket < 0)
		{
			std::cerr << "Failed to accept connection.\n";
            continue;
		}

		std::cout << "Connection accepted.\n";

		Client cl(new_socket, _conf);
		cl.print();
		// std::cout << "hello1" << std::endl;
		// req.print();


		static bool send_html = true;
		std::string filename = send_html ? "/home/zyko/webserv/landing_page/index.html" : "/home/zyko/webserv/landing_page/css/style.css";
		std::string content_type = send_html ? "text/html" : "text/css";
		std::string webpage = readFile(filename);
		std::string response = "HTTP/1.1 200 OK\nContent-Type: " + content_type + "\n\n" + webpage + "\n";
		write(new_socket, response.c_str(), response.length());
		send_html = !send_html;
		// exit(1);
		close(new_socket);
		// close(server_fd);
	}
}


#endif