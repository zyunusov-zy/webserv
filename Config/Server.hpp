#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include "Config.hpp"
// to set up a server
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <fstream>


# define RED "\033[31m"
# define NORMAL "\033[0m"

class Server
{
private:
	Config  _conf;

	std::string readFile(const std::string & filename);
	std::string readRequest(int socket);
	bool parseRequest(std::string request);
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

std::string Server::readRequest(int socket)
{
	std::string req;
	char buf[1024];
	ssize_t bytes;

	while((bytes = read(socket, buf ,sizeof(buf) - 1)) > 0)
	{
		buf[bytes] = '\0';
		req += buf;
		if (req.find("\r\n\r\n") != std::string::npos || req.find("\n\n") != std::string::npos)
			break;
	}

	if (bytes < 0)
	{
		std::cerr << "Failed to read from socket. \n";
		exit(1);
	}
	return req;

}

bool Server::parseRequest(std::string request)
{
	std::istringstream sstream(request);

	std::string request_line;

	if (!std::getline(sstream, request_line))
	{
		std::cerr << "Failed to read request line\n";
		return false;
	}

	std::string method;
	std::string resource;
	std::string version;

	std::istringstream rlstream(request_line);

	if (!(rlstream >> method >> resource >> version)) {
        std::cerr << "Failed to parse request line\n";
        return false;
    }
	if (method.empty() || resource.empty() || version.empty()) {
        std::cerr << "Invalid request line\n";
        return false;
    }

    std::cout << "Method: " << method << "\n";
    std::cout << "Resource: " << resource << "\n";
    std::cout << "Version: " << version << "\n";

    std::map<std::string, std::string> headers;
    std::string header_line;
    while (std::getline(sstream, header_line) && header_line != "\r") {
        header_line.pop_back(); // remove the '\r'
        size_t separator = header_line.find(": ");
        if (separator == std::string::npos) {
            std::cerr << "Invalid header line: no separator found\n";
            return false;
        }

        std::string name = header_line.substr(0, separator);
        std::string value = header_line.substr(separator + 2);
        if (name.empty() || value.empty()) {
            std::cerr << "Invalid header line: empty name or value\n";
            return false;
        }

        headers[name] = value;
    }

    for (auto header : headers) {
        std::cout << "Header: " << header.first << " = " << header.second << "\n";
    }

    return true;
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

		std::string request = readRequest(new_socket);
		parseRequest(request);


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