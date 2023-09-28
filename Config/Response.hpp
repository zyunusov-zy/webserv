#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
// #include "Server.hpp"

#define BUFF_SIZE 8192


class Response {
public:
    Response();

	void assembleHeader(); // needs to be created

    void setStatus(int statusCode);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
	void setFilename(std::string &name);
	void sendResponse(std::string content_type);
	void setFd(int fd);

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

private:

    int _statusCode;

    std::string _filename;
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


#endif // RESPONSE_H
