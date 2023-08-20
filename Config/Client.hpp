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
	bool _c;


	int _fdSock;
	bool _isClosed;
	bool _isRead;
	bool _toServe;
	std::string _clienIP;
	t_serv _serv;


	void parseEnvVar();
public:
	bool readRequest();
	Client(int new_socket, char *clien_ip,  t_serv s);
	~Client();
	void print();

	char **get_env();
	std::string	getClienIP();
	Request getReq();
	bool getQuer();

};

bool Client::getQuer()
{
	return (_quer);
}

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

Client::Client(int new_socket, char *clien_ip, t_serv s): _serv(s),
	 _req(_serv.loc), _c(false)
{
	// std::cout << std::endl << std::endl << "MBHERE?" << std::endl << std::endl;
	_clienIP = clien_ip;
	_fdSock = new_socket;
	_serv = s;
	_isClosed = false;
	_isRead = false;
	_toServe = false;
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
		_c = _req.checkCGI();
		if (_c)
			env_var = _req.getENV();
	}
	catch(ErrorException &e)
	{
		_toServe = true;
		std::cout << e.what() << std::endl;
		_req.setErrorStatus(e.getStatus());
	}
	return (_toServe);
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
}



#endif