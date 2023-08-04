#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <cerrno>
#include <cstring>
#include "Config.hpp"

# define BADREQUEST 400
# define OK 200;
# define REQUEST_ENTITY_TOO_LARGE 413;

typedef std::map<std::string, std::string> HeaderMap;

class Request 
{
	private:
		std::string _requestline;
		std::string _method;
		std::string _resource;
		std::string _version;
		HeaderMap _headers;
		HeaderMap _query;

		ssize_t receiveData(int socket, char* buffer, size_t length);
		int checkBodySize(size_t total_bytes_read, const Config& _conf, std::string h);
		std::string getHost();
		void parseQueryString(const std::string& query);
		void parseHeaders(std::istringstream& sstream);
		void parseRequestLine(const std::string& request_line);

	public:
		Request();
		~Request();
		int readRequest(int socket, Config _conf);
		int parseRequest();
		std::string& getMethod();
		std::string& getResource();
		std::string& getVersion();
		HeaderMap& getHeaders();
		void print();
		
};

Request::Request()
{
}

void Request::parseQueryString(const std::string& query)
{
    std::istringstream qstream(query);
    std::string param;
    while (std::getline(qstream, param, '&')) {
        size_t separator = param.find("=");
        if (separator == std::string::npos) {
            throw std::runtime_error("Invalid query string: no separator found");
        }
        std::string name = param.substr(0, separator);
        std::string value = param.substr(separator + 1);
        if (name.empty() || value.empty())
        {
            throw std::runtime_error("Invalid query string: empty name or value");
        }
        _query[name] = value;
    }
}

void Request::parseRequestLine(const std::string& request_line)
{
    std::istringstream rlstream(request_line);
    if (!(rlstream >> this->_method >> this->_resource >> this->_version))
    {
        std::cerr << "Failed to parse request line\n";
        throw std::runtime_error("Invalid request line");
    }

	if (_method.empty() || _resource.empty() || _version.empty()) {
        std::cerr << "Failed to parse request line\n";
        throw std::runtime_error("Invalid request line");
    }
}

void Request::parseHeaders(std::istringstream& sstream)
{
    std::string header_line;
    while (std::getline(sstream, header_line) && header_line != "\r") {
        header_line.pop_back(); // remove the '\r'
        size_t separator = header_line.find(": ");
        if (separator == std::string::npos) {
            throw std::runtime_error("Invalid header line: no separator found");
        }
        std::string name = header_line.substr(0, separator);
        std::string value = header_line.substr(separator + 2);
		if (name.empty() || value.empty()){
			throw std::runtime_error("Invalid header line: empty name or value\n");
		}
        _headers[name] = value;
    }
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

	try {
		parseRequestLine(request_line);

		std::string path = _resource;
		std::string query;
		size_t pos = _resource.find('?');
		if (pos != std::string::npos)
		{
			path = _resource.substr(0, pos);
			query = _resource.substr(pos + 1);
		}

		if (!query.empty()) {
				parseQueryString(query);
		}
		parseHeaders(sstream);
	}catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        return BADREQUEST;
    }
	return OK;
}

ssize_t Request::receiveData(int socket, char* buffer, size_t length)
{
    ssize_t bytes_received = recv(socket, buffer, length - 1, 0);

    if (bytes_received < 0) {
        std::cerr << "Error reading from socket: " << std::strerror(errno) << std::endl;
    } else if (bytes_received == 0) {
        std::cerr << "Client closed connection" << std::endl;
    }

    return bytes_received;
}

std::string Request::getHost()
{
    std::string hostHeader = "Host: ";
    size_t startPos = _requestline.find(hostHeader);
    if (startPos == std::string::npos) {
        std::cerr << "Host header not found in the request.\n";
        return "";
    }
    startPos += hostHeader.length(); // Skip past "Host: "
    size_t endPos = _requestline.find("\r\n", startPos);
    if (endPos == std::string::npos) {
        // Handle case where "\r\n" is not found after "Host: ".
        return "";
    }
    return _requestline.substr(startPos, endPos - startPos);
}

int Request::checkBodySize(size_t total_bytes_read, const Config& _conf, std::string h)
{
    size_t maxBody = 0;
	for (int i = 0; i < _conf.servers.size(); i++) {
		if (h == _conf.servers[i].ipPort || h == _conf.servers[i].namePort) {
			maxBody = _conf.servers[i].limit_client_size;
		}
	}
	// std::cout << maxBody << std::endl << std::endl;
    if (maxBody != -1 && total_bytes_read > maxBody) {
        std::cerr << "Client request body exceeded the maximum allowed size\n";
        return REQUEST_ENTITY_TOO_LARGE;
    }
	return OK;
}



int Request::readRequest(int socket, Config _conf)
{
	char buf[4096];
	bool chunked = false;
	int error_code = OK;
	size_t total_bytes_read = 0;

	while(true)
	{
		std::memset(buf, 0, sizeof(buf));
		ssize_t bytes_rec = receiveData(socket, buf, sizeof(buf));

		if (bytes_rec < 0)
		{
			std::cerr << "Error reading from socket: " << std::strerror(errno) << std::endl;
			error_code = BADREQUEST;
			break;
		}
		else if (bytes_rec == 0)
		{
			std::cerr << "Client closed connection" << std::endl;
			break;
		}
		total_bytes_read += bytes_rec;
		this->_requestline.append(buf, bytes_rec);

		std::string host = getHost();
		// std::cout << "Helloooo" <<std::endl;
        if (host.empty()) {
            error_code = BADREQUEST;
            std::cerr << "Host header not found or malformed in the request.\n";
            break;
        }
		error_code = checkBodySize(total_bytes_read ,_conf, host);
		if (error_code != 200) {
            return error_code;
        }
		if (!chunked)
		{
			if (_requestline.find("Transfer-Encoding: chunked") != std::string::npos)
				chunked = true;
			else if (_requestline.find("\r\n\r\n") != std::string::npos)
				break;
		}
		else if (chunked && _requestline.find("\r\n0\r\n\r\n") != std::string::npos)
			break;
		
	}
	return (error_code);
}

void Request::print()
{
	std::cout << "Method: " << getMethod() << "\n";
    std::cout << "Resource: " << getResource() << "\n";
    std::cout << "Version: " << getVersion() << "\n";

	HeaderMap tmp = getHeaders();
	for (auto header : tmp) {
        std::cout << header.first << " = " << header.second << "\n";
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