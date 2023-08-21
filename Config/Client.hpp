#ifndef CLIENT_HPP
# define CLIENT_HPP

// #include <iostream>
// #include <map>
// #include <string>
// #include "Request.hpp"
// #include "Response.hpp"
// #include "ErrorCodes.hpp"
#include "mainIn.hpp"
#include "Request.hpp"

class Request;

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
	
	Response _resp;

public:
	bool readRequest();
	Client(int new_socket, char *clien_ip,  t_serv s);
	~Client();
	void print();

	char **get_env();
	std::string	getClienIP();
	Request getReq();
	bool getQuer();
	Response &getResp();

};

#endif