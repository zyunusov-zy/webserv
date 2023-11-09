#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Response.hpp"
#include "mainIn.hpp"
#include "Request.hpp"
#include <dirent.h>

class Request;

class Client
{
private:
	Request _req;
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
	std::string fileList(std::string path, int &err, const Location *loc);

public:

	class returnClientError : public std::exception
	{
		public:
			virtual const char *what() const throw()
			{
				return ("___exception___\n");
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
	bool getToServe();


	int _statusCode;
	int fd;
	std::string	filename;


	pollfd	*pollstruct;

};

#endif
