#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <vector>
#include <sstream>

#define BUF_SIZE 1024



class Response {
public:
    Response();

    void setStatus(int statusCode);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
	void assembleHeader();


    std::string toString() const;

private:
    int _statusCode;
    std::string _statusMessage;
	std::string _header;
    // std::vector<std::pair<std::string, std::string> > _headers;
    std::string _body;

	std::string chunking;
	std::string	content_type;
	std::string	date;
	std::string	protocoll;
	std::string	status_code;
};

Response::Response() : _statusCode(200), _statusMessage("OK") {}

void Response::setStatus(int statusCode) {
    _statusCode = statusCode;
}

void Response::setHeader(const std::string& key, const std::string& value) {
    _headers.push_back(std::make_pair(key, value));
}

void Response::setBody(const std::string& body) {
    _body = body;
}

std::string Response::toString() const {
    std::stringstream response;

    response << "HTTP/1.0 " << _statusCode << " " << _statusMessage << "\r\n";

    for (size_t i = 0; i < _headers.size(); ++i) {
        response << _headers[i].first << ": " << _headers[i].second << "\r\n";
    }

    response << "\r\n" << _body;

    return response.str();
}



void assembleHeader()
{

	std::ostringstream tmp;

	if (BUF_SIZE < content_l)
	{
		additional_info = "Transfer-Encoding: chunked";
		is_chunked = true;
	}

	this->setDate();
	tmp << protocoll << " " << status_code << "\r\n";
	tmp << "Server: " << server_name << "\r\n";
	tmp << date << "\r\n";
	if (content_length)
	{
		tmp << "Content-Type: " << content_type << "\r\n";
		tmp << "Content-Length: " << content_length << "\r\n";
	}
	if (!additional_info.empty())
		tmp << additional_info << "\r\n";
	tmp << "\r\n";

	_header = tmp.str();
}



#endif // RESPONSE_H
