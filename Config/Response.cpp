#include "Response.hpp"

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

int calculateContentLength(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        int length = static_cast<int>(file.tellg());
        file.seekg(0, std::ios::beg);
        return length;
    }

void Response::sendResponse(std::string content_type) 
{

	std::ifstream file(_filename.c_str());

    if (!file.is_open()) {
    std::cerr << "Failed to open " << _filename << std::endl;
    }
    std::cerr << "\n in response" << _filename << "\n";

	char buff[BUFF_SIZE];

    char header[400];


//the header needs to be sent as a part of a chunk, 
    snprintf(buff, sizeof(buff), "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: %s\r\n"
                                         "Content-Length: %d\r\n"
                                         "Connection: keep-alive\r\n"
                                         "\r\n", content_type.c_str(), calculateContentLength(file));
    
	send(_target_fd, buff, strlen(buff), 0);


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
