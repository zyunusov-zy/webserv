#ifndef CLIENT_HPP
# define CLIENT_HPP

#define BUFF_SIZE 1020

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
	
	// Response _resp;
	int _errBodySize;

	std::string	genErrPage(int err);
	std::string errorMap(int err);
	void sendErrHTML(std::string _errPage, int err);

public:
	bool readRequest();
	Client(int new_socket, char *clien_ip,  t_serv s);
	~Client();
	void print();

	char **get_env();
	std::string	getClienIP();
	Request getReq();
	bool getQuer();
	// Response &getResp();
	int checkError();
	std::string checkErrorMap(int err);

	// void sendResponse(std::string content_type);


    // void setStatus(int statusCode);
    // void setHeader(const std::string& key, const std::string& value);
    // void setBody(const std::string& body);
	// void setFilename(std::string &name);
	void sendResponse(std::string content_type);
	// void setFd(int fd);

	int _statusCode;
	int fd;
	std::string	filename;


	pollfd	*pollstruct;

	std::string	status_code;
	std::string	content_type;
	size_t	content_len;
	std::string	additional_info;

	std::string header;
	std::streampos	position;
	bool 	header_sent;
	bool	response_complete;
	bool 	is_chunked;
    std::string body;

	std::string _header;
    std::string _body;
    std::string _status;
	int _target_fd;

	std::string	_date;
	std::string	_proto;
	std::string	_status_code;
	std::string _chunking;
	std::string	_content_type;
};

#endif