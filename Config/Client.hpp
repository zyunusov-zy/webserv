#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <map>
#include <string>
#include "Request.hpp"
#include "ErrorCodes.hpp"

class Client
{
private:
	Request _req;
	int error_code;
	char **env_var;
	bool _quer;


	int _fdSock;
	bool _isClosed;
	bool _isRead;
	bool _toServe;
	std::string _clienIP;
	t_serv _serv;
	std::map<std::string, std::string> _clientHeaders;
	std::vector<std::string> _env_vec;


	void parseEnvVar();
public:
	bool readRequest();
	Client(int new_socket, char *clien_ip,  t_serv s);
	~Client();
	void print();

	char **get_env();
	std::string	getClienIP();
	Request getReq();
};

Request Client::getReq()
{
	return (_req);
}

char **Client::get_env()
{
	return (env_var);
}

std::string	Client::getClienIP()
{
	return (_clienIP);
}

Client::Client(int new_socket, char *clien_ip, t_serv s): _serv(s), _req(_serv.loc)
{
	// std::cout << std::endl << std::endl << "MBHERE?" << std::endl << std::endl;
	_clienIP = clien_ip;
	_fdSock = new_socket;
	_serv = s;
	_isClosed = false;
	_isRead = false;
	_toServe = false;
	// std::cout << "File descriptor: " << new_socket << std::endl;
	// if (error_code == 200)
	// {
	// 	error_code = _req.parseRequest();
	// 	_quer = _req.getQ();
	// 	_clientHeaders = _req.getHeaders();
	// 	parseEnvVar();
	// }
}


bool Client::readRequest()
{
	ssize_t res = recv(_fdSock, _req.getBuffer(), RECV_BUFFER_SIZE, 0);
		// std::cout << "Hello" << std::endl;
	if (res == 0)
	{
		_isClosed = true;
		return (false);
	}
	else if (res < -1)
		throw ErrorException(502, "ERROR RECV DATA");
	_isRead = true;
	try
	{
		_toServe = _req.saveRequestData(res);
	}
	catch(ErrorException &e)
	{
		_toServe = true;
		std::cout << e.what() << std::endl;
		_req.setErrorStatus(e.getStatus());
	}
	return (_toServe);
}

void Client::parseEnvVar()
{
	std::string tmp;
	if(!_quer)
	{
		if (_req.getMethod() == "POST") // will add it only with POST
		{
			tmp = "CONTENT_LENGTH=" + _clientHeaders["Content-Length:"];
			_env_vec.push_back(tmp);
			tmp = "CONTENT_TYPE=" + _clientHeaders["Content-Type:"];
			_env_vec.push_back(tmp);
		}
		tmp = "PATH_INFO=" + _req.getPathInfo();
		_env_vec.push_back(tmp);
	}
	else
	{
		tmp = "QUERY_STRING=" + _req.getQueryString();// need to ask do you need full query string
		_env_vec.push_back(tmp);
	}
	tmp = "GATEWAY_INTERFACE=CGI/1.1";
	_env_vec.push_back(tmp);
	tmp = "REQUEST_METHOD=" + _req.getMethod();
	_env_vec.push_back(tmp);
	tmp = "REMOTE_ADDR=" + _clienIP; // need to know where to get Client IP
	_env_vec.push_back(tmp);
	tmp = "SCRIPT_NAME=" + _req.getScriptName();
	_env_vec.push_back(tmp);
	tmp = "SERVER_NAME=" + _req.getServ().name;
	_env_vec.push_back(tmp);
	tmp = "SERVER_PORT=" + std::to_string(_req.getPortOfReq());
	_env_vec.push_back(tmp);
	tmp = "SERVER_PROTOCOL=HTTP/1.1";
	_env_vec.push_back(tmp);
	tmp = "SERVER_SOFTWARE=Webserver";
	_env_vec.push_back(tmp);

	// for(size_t i = 0; i < _env_vec.size(); i++)
	// {
	// 	std::cout << _env_vec[i] <<std::endl;
	// }
	env_var = new char*[_env_vec.size() + 1];
	for(size_t i = 0; i < _env_vec.size(); ++i)
	{
		env_var[i] = new char[_env_vec[i].size() + 1];
		std::strcpy(env_var[i], _env_vec[i].c_str());
	}
	env_var[_env_vec.size()] = NULL;
}

void Client::print()
{
	_req.print();
	std::cout << "Error_code: " << _req.getErrorCode() << std::endl;
	std::cout << "Query: " << _req.getQ() << std::endl;
	std::cout << "Server host: " << _serv.host << std::endl;
	// std::cout << "hello" << std::endl;
	// for (int i = 0; env_var[i] != NULL; ++i)
	// {
	// 	std::cout << env_var[i] << std::endl;
	// }
	// std::cout << "hello222" << std::endl;
}

Client::~Client()
{
	// for(size_t i = 0; i < _env_vec.size(); ++i)
	// {
	// 	delete[] env_var[i];
	// }
	// delete[] env_var;
}



#endif