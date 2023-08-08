#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <cerrno>
#include <cstring>
#include <fstream>
#include "Config.hpp"
#include "Server.hpp"
#include "ErrorCodes.hpp"


typedef std::map<std::string, std::string> HeaderMap;

class Config;

class Request 
{
	private:
		std::string _requestline;
		std::string _method;
		std::string _resource;
		std::string _version;
		std::string	_scriptName;
		std::string _pathInfo;
		std::string _queryString;
		int			_portOfReq;
		HeaderMap _headers;
		HeaderMap _query;
		std::string _body;
		t_serv serv;
		int _err;
		bool _q;

		ssize_t receiveData(int socket, char* buffer, size_t length);
		int initServ(const Config& _conf, std::string h);
		std::string getHost();
		void parseQueryString();
		void parseHeaders(std::istringstream& sstream);
		void parseRequestLine(const std::string& request_line);
		bool doesFileExist(const std::string& filePath);
		void parseResource();
		void parseBody();
	public:
		Request();
		~Request();
		bool getQ();
		int readRequest(int socket, Config _conf);
		int parseRequest();
		std::string& getMethod();
		std::string& getResource();
		std::string& getVersion();
		std::string& getScriptName();
		std::string& getPathInfo();
		std::string& getBody();
		std::string& getQueryString();
		int getPortOfReq();
		t_serv getServ();
		HeaderMap& getHeaders();
		void print();
		
};

Request::Request(): _q(false), _err(0)
{
}

bool Request::getQ()
{
	return _q;
}

int Request::getPortOfReq()
{
	return _portOfReq;
}

std::string& Request::getScriptName()
{
	return _scriptName;
}

std::string& Request::getPathInfo()
{
	return _pathInfo;
}

std::string& Request::getBody()
{
	return _body;
}

std::string& Request::getQueryString()
{
	return _queryString;
}

t_serv Request::getServ()
{
	return serv;
}

bool Request::doesFileExist(const std::string& filePath) {
    std::ifstream infile(filePath.c_str());
    return infile.good();
}

void Request::parseQueryString()
{
    std::istringstream qstream(_queryString);
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
	_q = true;
}

void Request::parseResource()
{
	std::string scriptDir = "/cgi-bin";
	size_t pos = _resource.find(scriptDir);

	if (pos != std::string::npos) {
		// Handle as a CGI request
		pos += scriptDir.length();
		std::string remainingPath = _resource.substr(pos);
		std::istringstream ss(remainingPath);
		std::string segment;
		while (std::getline(ss, segment, '/')) {
			if (doesFileExist(scriptDir + '/' + segment)) {
				_scriptName = scriptDir + '/' + segment;
				// Find the position of scriptName in resource to split the pathInfo
				pos = _resource.find(_scriptName);
				pos += _scriptName.length();
				if (pos < _resource.length()) {
					_pathInfo = _resource.substr(pos);
				} else {
					_pathInfo = "";
				}
				return;
			}
		}
		throw std::runtime_error("Invalid resource path: " + _resource);
	} else {
		// Non-CGI request, treat as a file to serve
		_scriptName = _resource;
		_pathInfo = _resource;
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
	try{
		parseResource();
	}catch (const std::runtime_error& e) {
        std::cerr << "Failed to parse resource: " << e.what() << "\n";
		_err = 1;
        throw;  // Re-throw the exception
    }
}

void Request::parseBody()
{
	std::map<std::string, std::string>::iterator it = _headers.find("Content-Length");
	if (it != _headers.end())
	{
		// The 'Content-Length' header exists
		int content_length = std::stoi(it->second);
		if (content_length > serv.limit_client_size)
			throw std::runtime_error("Request Body is too Large!");
		size_t body_start_pos = _requestline.find("\r\n\r\n");

		if (body_start_pos != std::string::npos) {
			body_start_pos += 4;
			_body = _requestline.substr(body_start_pos, content_length);
		} else {
			_body = "";
		}
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
	try
	{
		parseBody();
	}catch(const std::exception& e)
	{
		std::cerr << "Failed to parse body: " << e.what() << "\n";
		_err = 2;
        throw;  // Re-throw the exception
	}
	
}

int Request::parseRequest()
{
	std::istringstream sstream(_requestline);
	std::string request_line;
	if (!std::getline(sstream, request_line))
	{
		std::cerr << "Failed to read request line\n";
		return BADREQUEST;// do i need to return?
	}

	try {
		parseRequestLine(request_line);
		std::string path = _resource;
		size_t pos = _resource.find('?');
		if (pos != std::string::npos)
		{
			path = _resource.substr(0, pos);
			_queryString = _resource.substr(pos + 1);
		}

		if (!_query.empty()) {
				parseQueryString(); // if we dont need map delete
		}
		parseHeaders(sstream);
	}catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
		_q = false;
		if (_err == 1)
			return UNABLE_TO_FIND;
		if (_err == 2)
			return REQUEST_ENTITY_TOO_LARGE;
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
    }// need to fix

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

int Request::initServ(const Config& _conf, std::string h)
{
	bool check = true;
	for (int i = 0; i < _conf.servers.size(); i++) {
		if (_conf.servers[i].ipPort.find(h) != std::string::npos 
		|| _conf.servers[i].namePort.find(h) != std::string::npos) {
			serv = _conf.servers[i];
			check = false;
		}
	}
	if (check)
	{
		std::cerr << "Request done for the unkown ip/port or server_nam/port" << std::endl;
		return BADREQUEST;
	}
	for(int i = 0; i < serv.port.size(); i++)
	{
		if (h.find(std::to_string(serv.port[i])))
			_portOfReq = serv.port[i];

	}
	return OK;
}



int Request::readRequest(int socket, Config _conf)
{
	char buf[4096];
	bool chunked = false;
	int error_code = OK;

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
		this->_requestline.append(buf, bytes_rec);

		std::string host = getHost();
		// std::cout << "Helloooo" <<std::endl;
        if (host.empty()) {
            error_code = BADREQUEST;
            std::cerr << "Host header not found or malformed in the request.\n";
            break;
        }
		error_code =initServ(_conf, host);
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
	std::cout << "PATH_INFO: " << getPathInfo() << "\n";
	std::cout << "ScriptName: " << getScriptName() << "\n";
	std::cout << "QueryString: " << getQueryString() << "\n" << "\n";

	std::cout << "Body: " << getBody() << "\n" << "\n";


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