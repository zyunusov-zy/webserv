#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>

# define BADREQUEST 400
# define OK 200;

typedef std::map<std::string, std::string> HeaderMap;

class Request 
{
	private:
		std::string _requestline;
		std::string _method;
		std::string _resource;
		std::string _version;
		HeaderMap _headers;

		int parseRequest();
	public:
		Request();
		~Request();
		int readRequest(int socket);
		std::string& getMethod();
		std::string& getResource();
		std::string& getVersion();
		HeaderMap& getHeaders();
		void print();
		
};

Request::Request()
{
}

int Request::parseRequest()
{
	std::istringstream sstream(_requestline);
	std::string request_line;
	if (!std::getline(sstream, request_line))
	{
		std::cerr << "Failed to read request line\n";
		return BADREQUEST;
	}

	std::istringstream rlstream(request_line);
	if (!(rlstream >> this->_method >> this->_resource >> this->_version))
	{
		std::cerr << "Failed to parse request line\n";
        return BADREQUEST;
	}
	if (_method.empty() || _resource.empty() || _version.empty()) {
        std::cerr << "Invalid request line\n";
        return BADREQUEST;
    }
	std::string header_line;
	while (std::getline(sstream, header_line) && header_line != "\r") {
		header_line.pop_back(); // remove the '\r'
		size_t separator = header_line.find(": ");
		if (separator == std::string::npos) {
            std::cerr << "Invalid header line: no separator found\n";
            return BADREQUEST;
        }
		std::string name = header_line.substr(0, separator);
		std::string value = header_line.substr(separator + 2);
		if (name.empty() || value.empty())
		{
			std::cerr << "Invalid header line: empty name or value\n";
            return BADREQUEST;
		}
		_headers[name] = value;
	}

	return OK;
}

int Request::readRequest(int socket)
{
	char buf[1024];
	ssize_t bytes;

	while((bytes = read(socket, buf ,sizeof(buf) - 1)) > 0)
	{
		buf[bytes] = '\0';
		this->_requestline += buf;
		if (_requestline.find("\r\n\r\n") != std::string::npos || _requestline.find("\n\n") != std::string::npos)
			break;
	}

	if (bytes < 0)
	{
		std::cerr << "Failed to read from socket. \n"; // error code 400
		return BADREQUEST;
	}
	int error_code = parseRequest();
	return (error_code);
}

void Request::print()
{
	std::cout << "Method: " << getMethod() << "\n";
    std::cout << "Resource: " << getResource() << "\n";
    std::cout << "Version: " << getVersion() << "\n";

	HeaderMap tmp = getHeaders();
	for (auto header : tmp) {
        std::cout << "Header: " << header.first << " = " << header.second << "\n";
    }
}

std::string& Request::getMethod()
{
	return this->_method;
}
std::string& Request::getResource()
{
	return this->_resource;
}
std::string& Request::getVersion()
{
	return this->_version;
}
HeaderMap& Request::getHeaders()
{
	return this->_headers;
}

Request::~Request()
{
}
#endif