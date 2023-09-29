#include "Response.hpp"

Response::Response() 
{
    // _statusCode = 200;

    status_code = "200 OK";
	is_chunked = false;
    _proto = "HTTP/1.1";
    position = 0;
    additional_info = "";
    response_complete = false;
    header_sent = false;

    header = "";
    body = "";
    _status = "";
    _chunking = "";
    content_type = "";
    _date = "";
    status_code = "";
	_filename = "";
	_target_fd = 0;

}

// Response::Response() 
// {
//     _statusCode = 200;
//     // _header = "";
//     // _body = "";
//     // _status = "";
//     // _chunking = "";
//     // _content_type = "";
//     // _date = "";
//     // _proto = "";
//     // _status_code = "";
// 	// _filename = "";
// 	// _target_fd = 0;

// }

int calculateContentLength(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        int length = static_cast<int>(file.tellg());
        file.seekg(0, std::ios::beg);
        return length;
    }

void Response::sendResponse(std::string content_type) 
{
    std::cerr << "\n in response" << _filename << "\n";
    std::string chunk = "";

    // std::ifstream file(_filename.c_str());

    char buff[BUFF_SIZE];
    std::string head;
    
    std::ifstream file(_filename.c_str(), std::ios::binary);
    if (!file.is_open()) 
    {
        std::cerr << "Failed to open " << _filename << std::endl;
    }
    if (!header_sent)
    {
        // createHeader();
        // chunk += header;

        snprintf(buff, sizeof(buff), "HTTP/1.1 200 OK\r\n"
                                         "Content-Type:  %s\r\n"
                                         "Content-Length: %d\r\n"
                                         "Connection: close\r\n"
                                        //  "Transfer-Encoding: chunked\r\n"
                                         "\r\n", content_type.c_str(), calculateContentLength(file));

        // snprintf(buff, sizeof(buff), "HTTP/1.1 200 OK\r\n"
        //                                  "Content-Type: %s\r\n"
        //                                  "Content-Length: %d\r\n"
        //                                  "Connection: keep-alive\r\n"
        //                                  "Transfer-Encoding: chunked\r\n"
        //                                  "\r\n", "multipart/form-data", calculateContentLength(file));

                                        //  "\r\n", content_type.c_str(), calculateContentLength(file));
    
	    send(_target_fd, buff, strlen(buff), 0);
        header_sent = true;
    }

	// std::ifstream file(_filename.c_str(), std::ios::binary);


	// char buff[BUFF_SIZE];

    file.seekg(position);
    file.read(buff, sizeof(buff));
    std::streampos currentPos = file.tellg();


    int bytesRead = file.gcount();
    if (bytesRead > 0) 
    {
        std::cerr << "*******READING " << _filename << std::endl;

        int bytesSent = send(_target_fd, buff, bytesRead, 0);
        if (bytesSent < 0) {
            std::cerr << "Send error" << std::endl;
        }
        // pollstruct->events = POLLOUT;
		response_complete = false;


    }
    else if (file.eof() || bytesRead == 0)
	{
        std::cerr << "*******EOF " << _filename << std::endl;

		file.close();
		response_complete = true;
        // pollstruct->revents = POLLOUT;
        std::cerr << "all read " << _filename << std::endl;

	}
	// close(_target_fd);
}

// void Response::sendResponse(std::string content_type) 
// {

// 	std::ifstream file(_filename.c_str());

//     if (!file.is_open()) {
//     std::cerr << "Failed to open " << _filename << std::endl;
//     }
//     std::cerr << "\n in response" << _filename << "\n";

// 	char buff[BUFF_SIZE];

//     char header[400];


// //the header needs to be sent as a part of a chunk, 
//     snprintf(buff, sizeof(buff), "HTTP/1.1 200 OK\r\n"
//                                          "Content-Type: %s\r\n"
//                                          "Content-Length: %d\r\n"
//                                          "Connection: keep-alive\r\n"
//                                          "\r\n", content_type.c_str(), calculateContentLength(file));
    
// 	send(_target_fd, buff, strlen(buff), 0);


// 	while (file.good())
//     {
//         file.read(buff, sizeof(buff));
//         int bytesRead = file.gcount();
//         if (bytesRead > 0) 
// 		{
//             int bytesSent = send(_target_fd, buff, bytesRead, 0);
//             if (bytesSent < 0) {
//                 std::cerr << "Send error" << std::endl;
//                 break;
//             }
//         }
//         else 
//         {
//             // No more data to send
//             break;
//         }
// 	}
// 	// close(_target_fd);
// }


// void	Response::createHeader()
// {
// 	std::ostringstream tmp;

// 	if (BUF_SIZE < content_length)
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

// 	header = tmp.str();
// }


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
