#ifndef CLIENT_HPP
# define CLIENT_HPP

// #include <iostream>
// #include <map>
// #include <string>
// #include "Request.hpp"
#include "Response.hpp"
// #include "ErrorCodes.hpp"
#include "mainIn.hpp"
#include "Request.hpp"
#include <dirent.h>

class Request;

class Client
{
private:
	Request _req;
	// int error_code;
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
	
	// Response _resp;
	int _errBodySize;

	std::string	genErrPage(int err);
	std::string errorMap(int err);
	void sendErrHTML(std::string _errPage, int err);
	// static std::string buildPath(std::string &path, const Location *loc, std::string fileName);
	std::string fileList(std::string path, int &err, const Location *loc);

public:

	class returnClientError : public std::exception
	{
		public:
			virtual const char *what() const throw()
			{
				return ("exception: CGI\n");
			}
	};
	int error_code;
	bool exec_err;
	int	exec_err_code;

	Response *_resp;
	bool readRequest();
	Client(int new_socket, char *clien_ip,  t_serv s);
	~Client();
	void print();

	char **get_env();
	std::string	getClienIP();
	Request getReq();
	bool getQuer();
	Response *getResp();
	int checkError();
	std::string checkErrorMap(int err);
	t_serv getServ();
	bool getIsClosed();


	// void sendResponse(std::string content_type);

	int _statusCode;
	int fd;
	std::string	filename;


	pollfd	*pollstruct;

};

#endif