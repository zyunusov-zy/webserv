#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <vector>
#include <sstream>

#define BUFF_SIZE 8192



class Response {
public:
    Response();

    void setStatus(int statusCode);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
	void assembleHeader();
	void setFilename(std::string &name);
	void sendResponse();
	void setFd(int fd);


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


Response::Response() 
{
    _statusCode = 200;
    // _header = "";
    // _body = "";
    // _status = "";
    // _chunking = "";
    // _content_type = "";
    // _date = "";
    // _proto = "";
    // _status_code = "";
	// _filename = "";
	// _target_fd = 0;

}

void Response::sendResponse() 
{

	std::ifstream file(_filename.c_str());
    std::cerr << "\n in response" << _filename << "\n";

	char buff[BUFF_SIZE];

    char header[400];

    // std::ifstream outFileStream(_filename.c_str());

    // if (!outFileStream) {
    //     std::cerr << "Failed to open output file for reading." << std::endl;
    // }
    // std::cerr << "\n" << 5 << "\n";

    // std::string output;
    // std::string line;
    // while (std::getline(outFileStream, line)) {
    //     output += line + "\n";
    // }

    snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", "text/html");

    send(_target_fd, header, strlen(header), 0);
    // send(_target_fd, output.c_str(), output.size(), 0);

    // outFileStream.close();
    // remove(_filename.c_str);
    // close(fd); // Close the client socket


	while (file.good())
    {
        file.read(buff, sizeof(buff));
        int bytesRead = file.gcount();
        if (bytesRead > 0) 
		{
            int bytesSent = send(_target_fd, buff, bytesRead, 0);
            if (bytesSent < 0) {
                std::cerr << "Send error" << std::endl;
                break;
            }
        }
        else 
        {
            // No more data to send
            break;
        }
	}
	// close(_target_fd);
}

void Response::setFd(int fd) {
    _target_fd = fd;
}

void Response::setFilename(std::string &name) {
    _filename = name;
    std::cout << "\n\nset name " << _filename << "\n";
}

void Response::setStatus(int statusCode) {
    _statusCode = statusCode;
}

// void Response::setHeader(const std::string& key, const std::string& value) {
//     _headers.push_back(std::make_pair(key, value));
// }

// void Response::setBody(const std::string& body) {
//     _body = body;
// }

// std::string Response::toString() const {
//     std::stringstream response;

//     response << "HTTP/1.0 " << _statusCode << " " << _statusMessage << "\r\n";

//     for (size_t i = 0; i < _headers.size(); ++i) {
//         response << _headers[i].first << ": " << _headers[i].second << "\r\n";
//     }

//     response << "\r\n" << _body;

//     return response.str();
// }



// void assembleHeader()
// {

// 	std::ostringstream tmp;

// 	if (BUF_SIZE < content_l)
// 	{
// 		additional_info = "Transfer-Encoding: chunked";
// 		is_chunked = true;
// 	}

// 	this->setDate();
// 	tmp << protocoll << " " << status_code << "\r\n";
// 	tmp << "Server: " << server_name << "\r\n";
// 	tmp << date << "\r\n";
// 	if (content_length)
// 	{
// 		tmp << "Content-Type: " << content_type << "\r\n";
// 		tmp << "Content-Length: " << content_length << "\r\n";
// 	}
// 	if (!additional_info.empty())
// 		tmp << additional_info << "\r\n";
// 	tmp << "\r\n";

// 	_header = tmp.str();
// }



#endif // RESPONSE_H
