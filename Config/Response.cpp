#include "Response.hpp"

Response::Response() 
{

	exec_err = false;
	exec_err_code = 0;

    status_code = "200 OK";
    _proto = "HTTP/1.1";
    position = 0;
    response_complete = false;
    header_sent = false;

    header = "";
    body = "";
    _status = "";
    _chunking = "";
    content_type = "";
    _date = "";
    status_code = "";
	filename = "";
	_target_fd = 0;

}

int calculateContentLength(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        int length = static_cast<int>(file.tellg());
        file.seekg(0, std::ios::beg);
        return length;
    }


bool Response::sendResponse(std::string content_type) 
{
    // std::cerr << "\n in response" << filename << "\n";
    std::string chunk = "";
	int	conl = 0;

    char buff[BUFF_SIZE];
    std::string head;
	std::ifstream file(filename.c_str(), std::ios::binary);
    std::cerr << "\n+++++++BODY CONTENT =======  " << std::endl;
	std::cerr << body << std::endl;


	if (body.length())
	{
		conl = body.length();
	}
	else
	{
		if (!file.is_open()) 
		{
			std::cerr << "Failed to open " << filename << std::endl;
		}
		conl = calculateContentLength(file);

	}

    if (!header_sent)
    {
        std::cerr << "\nSending header " << filename << std::endl;
        snprintf(buff, sizeof(buff), "HTTP/1.1 %\r\n"
                                         "Content-Type: %\r\n"
                                         "Content-Length: %d\r\n"
                                         "Connection: keep-alive\r\n"
                                         "\r\n", status_code.c_str(), content_type.c_str(), conl);
        std::cerr << "\nSending header " << content_type << std::endl;
        std::cerr << "\nSending header " << _target_fd << std::endl;



	    send(_target_fd, buff, strlen(buff), 0);
        header_sent = true;
    }

	if (body.length())
	{
		send(_target_fd, buff, strlen(buff), 0);
		response_complete = true;
		std::cerr << "\n out of sndinggggg " << std::endl;
		return 0;
	}

    file.seekg(position);
    file.read(buff, sizeof(buff));
    std::streampos currentPos = file.tellg();


    int bytesRead = file.gcount();
    if (bytesRead > 0) 
    {
        // std::cerr << "*******READING " << filename << std::endl;

        int bytesSent = send(_target_fd, buff, bytesRead, 0);
        if (bytesSent < 0) {
            std::cerr << "Send error" << std::endl;
			exec_err_code = 500;
			exec_err = true;
			response_complete = true;
			return 1;
			// checkError();
        }
		response_complete = false;
    }
	else if (file.eof() || bytesRead == 0)
	// else if (bytesRead == 0)
	{
        std::cerr << "*******EOF " << filename << std::endl;

		file.close();
		response_complete = true;
        std::cerr << "all read " << filename << std::endl;
	}
	position = file.tellg();
	return 0;
	// close(_target_fd);
}

void Response::setFd(int fd) {
    _target_fd = fd;
}

void Response::setFilename(std::string &name) {
    filename = name;
    std::cout << "\n\nset name " << filename << "\n";
}
