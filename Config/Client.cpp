#include "Client.hpp"

Response *Client::getResp()
{
	return (_resp);
}

bool Client::getQuer()
{
	return (_quer);
}

bool Client::getIsClosed()
{
	return (_isClosed);
}

bool Client::getToServe()
{
	return (_toServe);
}

// Request *Client::getPointReq()
// {
// 	return (&_req);
// }

Request Client::getReq()
{
	return (_req);
}

char **Client::get_env()
{
	return (env_var);
}

std::string	Client::getClienIP()
{
	return (_clienIP);
}

t_serv	Client::getServ()
{
	return (_serv);
}

Client::Client(int new_socket, char *clien_ip, t_serv s): _serv(s),
	 _req(_serv.loc), _c(false)
{
	// std::cout << std::endl << std::endl << "_*)*)*)*)*)*)Client constructor _*_*_*_*_*_*" << std::endl << std::endl;
	_clienIP = clien_ip;
	_fdSock = new_socket;
	_serv = s;
	_isClosed = false;
	_isRead = false;
	_toServe = false;
	_errBodySize = 0;

	exec_err_code = 0;
	exec_err = false;
}


bool Client::readRequest()
{
	ssize_t res = recv(_fdSock, _req.getBuffer(), RECV_BUFFER_SIZE, 0);
		// std::cout << "Hello" << std::endl;
	if (res == 0)
	{
		_isClosed = true;
		return (false);
	}
	else if (res < -1)
		throw ErrorException(502, "ERROR RECV DATA");
	_isRead = true;
	try
	{
		_toServe = _req.saveRequestData(res);
		_c = _req.checkCGI();
		if (_c)
			env_var = _req.getENV();
		if (_req.getCon())
			_isClosed = true;
		// std::cout << "ISCLOSED: " << _isClosed ? "TRUE" : "FALSE";
	}
	catch(ErrorException &e)
	{
		_toServe = true;
		std::cout << e.what() << std::endl;
		_req.setErrorStatus(e.getStatus());
	}
	return (_toServe);
}

std::string Client::checkErrorMap(int err)
{
	std::map<int, std::string> tmp = _serv.errorPages;
	typedef std::map<int, std::string>::iterator Mapiter;
	for(Mapiter it = tmp.begin(); it != tmp.end(); it++)
	{
		if (it->first == err)
		{
			std::ifstream inFile(it->second.c_str());
			if (!inFile)
			{
				std::cout << "Could not open an Error_page need to generate it" << std::endl;
				break;
			}
			return it->second;
		}
	}
	return "";
}

int  Client::checkError()
{

	int err;

	if (getResp()->exec_err_code > 0)
	{
		err = getResp()->exec_err_code;
		getResp()->exec_err_code = 0;
	}
	else
		err = _req.getErrorCode();



	std::cout << "ERRRORRRRR 00000000 " << err << std::endl;

	// std::cout << "ERRRRRRORRRR!!!:" << err << std::endl;
	if (err == 301 || err == 200 || err == 0 || err == 201)
		return 1;
	if (err == 1)
	{
		std::string urr = _req.getURI();
		if (urr[urr.length() - 1] != '/')
			urr.push_back('/');
		std::cout << urr << std::endl << std::endl;
		std::string indexAuto = fileList(urr, err, _req.getLocation());

		std::cout << indexAuto << std::endl << std::endl;
		if (err == 200)
		{
			std::cout << "HERE AUTOINDEX 2222222222222222222" << std::endl;
			sendErrHTML(indexAuto, err);
			return 0;
		}
			//call function to send html
	}
	std::string errorPath = "";
	errorPath = checkErrorMap(err);
	// std::cout << "Hhhhhhhhhhhhasdfjasfajskf" << errorPath << std::endl;
	if (errorPath != "")
	{
		_req.setResource(errorPath);
		return 1;
	}
	else if (errorPath == "")
	{
		// std::cerr << " MB HERE????? \n";
		// std::cout << "ERRRRRRORRRR!!!:" << err << std::endl;
		std::string _errPage = genErrPage(err);
		sendErrHTML(_errPage, err); // function to send generated error pages
	}
	return 0;
}

static std::string buildPath(std::string &path, const Location *loc, std::string fileName)
{
	std::string resPath;
	std::string tmp;
	size_t pos;

	std::cout << std::endl << std::endl;
	std::cout << "FILENAME: " << fileName << std::endl;
	std::cout << "PATH: " << path << std::endl;

	std::cout << "Location: " << loc->root << std::endl;
	pos = path.rfind(loc->root);
	std::cout << pos << std::endl;
	if (fileName == "." || fileName == ".." || pos == std::string::npos)
	{
		std::cout << "HERE1" << std::endl;
		return ".";
	}
	tmp = path.substr(pos + loc->root.length());
	if (!tmp.length())
		tmp = "/";
	if (tmp[0] != '/')
		tmp.insert(tmp.begin(), '/');
	pos = tmp.find(loc->path);
	std::cout << tmp << std::endl;
	if (pos == std::string::npos)
	{
		std::cout << "HERE2" << std::endl;
		return ".";
	}
	if (tmp == "/")
		return fileName;
	return (tmp.substr(pos) + "/" + fileName);
}

std::string Client::fileList(std::string path, int &err, const Location *loc)
{
	std::string htmlB;
	DIR  		*dirPtr; // It is used as a handle for directory operations such as opening a directory and reading its contents.
	struct dirent *dirent;
	std::string pathToFile;
	if (!loc)
	{
		err = 404;
		return NULL;
	}
	dirPtr = opendir(path.c_str());
	if (!dirPtr)
	{
		err = 403;
		return NULL;
	}

	htmlB = "<!DOCTYPE html>\n";
	htmlB += "<html>\n";
	htmlB += "<head><title>AutoIndexON</title></head>\n";
	htmlB += "<body>\n<h1>Files in current directory</h1>\n";
	dirent = readdir(dirPtr);
	while(dirent)
	{
		pathToFile = buildPath(path, loc, dirent->d_name);
		std::cout << "Path TO FILE: " << pathToFile << std::endl;
		if (pathToFile != ".")
		{
			htmlB  += "<div><a href=\"" + pathToFile + "\"><h2>"
				+ dirent->d_name + "</a></h2>\n";
		}
		dirent = readdir(dirPtr);
	}
	closedir(dirPtr);
	htmlB += "</body>\n</html>\n";
	err = 200;
	return htmlB;// need to count boySize while sendning
}

void Client::sendErrHTML(std::string _errPage, int err)
{
    std::string contentType = "text/html";
    int bodySize = _errPage.length();  // Corrected this line
    
    char buff[BUFF_SIZE];
    snprintf(buff, sizeof(buff), "HTTP/1.1 %d %s\r\n"
                                 "Content-Type: %s\r\n"
                                 "Content-Length: %d\r\n"
                                 "Connection: keep-alive\r\n"
                                 "\r\n",
                                 err, errorMap(err).c_str(), contentType.c_str(), bodySize);

    // Send the headers
    send(_fdSock, buff, strlen(buff), 0);

    // Send the error page content
    send(_fdSock, _errPage.c_str(), _errPage.size(), 0);  // Directly use the string
}

std::string Client::errorMap(int err)
{
	static std::map<int,std::string> errMap;
	if (!errMap.size())
	{
		errMap.insert(std::pair<int, std::string>(200, " Ok"));
		errMap.insert(std::pair<int, std::string>(201, " Created"));
		errMap.insert(std::pair<int, std::string>(204, " No Content"));
		errMap.insert(std::pair<int, std::string>(301, " Moved Permanently"));
		errMap.insert(std::pair<int, std::string>(400, " Bad Request"));
		errMap.insert(std::pair<int, std::string>(403, " Forbidden"));
		errMap.insert(std::pair<int, std::string>(404, " Not Found"));
		errMap.insert(std::pair<int, std::string>(409, " Conflict"));
		errMap.insert(std::pair<int, std::string>(405, " Method Not Allowed"));
		errMap.insert(std::pair<int, std::string>(413, " Payload Too Large"));
		errMap.insert(std::pair<int, std::string>(500, " Internal Server Error"));
		errMap.insert(std::pair<int, std::string>(502, " Bad Gateway"));
		errMap.insert(std::pair<int, std::string>(503, " Service Unavailable"));
	}
	return errMap[err];
}

std::string	Client::genErrPage(int err)
{
	// std::cerr << " MB HERE????? \n";
	std::stringstream buff;
	buff <<  "<html>\n";
	buff <<  "<head><title>" + std::to_string(err) + errorMap(err) + "</title></head>\n";
	buff <<  "<body>\n";
	buff <<	"<div><h1>" + std::to_string(err) + errorMap(err) + "</h1>\n";
	buff << "</body>\n";
	buff <<  "</html>\n";

	std::string res = buff.str();
	std::cout << res << std::endl;// test
	return res;
}

void Client::print()
{
	_req.print();
	std::cout << "Error_code: " << _req.getErrorCode() << std::endl;
	std::cout << "Query: " << _req.getQ() << std::endl;
	std::cout << "Server host: " << _serv.host << std::endl;
	// std::cout << "hello" << std::endl;
	// for (int i = 0; env_var[i] != NULL; ++i)
	// {
	// 	std::cout << env_var[i] << std::endl;
	// }
	// std::cout << "hello222" << std::endl;
}

// int calculateContentLength(std::ifstream& file) {
//         file.seekg(0, std::ios::end);
//         int length = static_cast<int>(file.tellg());
//         file.seekg(0, std::ios::beg);
//         return length;
//     }

// void Client::sendResponse(std::string content_type) 
// {
//     // std::cerr << "\n in response" << filename << "\n";
//     std::string chunk = "";
// 	int _target_fd = _fdSock;

//     char buff[BUFF_SIZE];
//     std::string head;
    
//     std::ifstream file(filename.c_str(), std::ios::binary);
//     if (!file.is_open()) 
//     {
//         std::cerr << "Failed to open " << filename << std::endl;
//     }
//     if (!header_sent)
//     {
//         // createHeader();
//         // chunk += header;
//         std::cerr << "\nSending header " << filename << std::endl;

//         snprintf(buff, sizeof(buff), "HTTP/1.1 200 OK\r\n"
//                                          "Content-Type: %\r\n"
//                                          "Content-Length: %d\r\n"
//                                          "Connection: keep-alive\r\n"
//                                         //  "Transfer-Encoding: chunked\r\n"
//                                          "\r\n", content_type.c_str(), calculateContentLength(file));

// 	    send(_target_fd, buff, strlen(buff), 0);
//         header_sent = true;
//     }

//     file.seekg(position);
//     file.read(buff, sizeof(buff));
//     std::streampos currentPos = file.tellg();


//     int bytesRead = file.gcount();
//     if (bytesRead > 0) 
//     {
//         // std::cerr << "*******READING " << filename << std::endl;

//         int bytesSent = send(_target_fd, buff, bytesRead, 0);
//         if (bytesSent < 0) {
//             std::cerr << "Send error" << std::endl;
//         }
// 		response_complete = false;
//     }
// 	else if (file.eof() || bytesRead == 0)
// 	// else if (bytesRead == 0)
// 	{
//         std::cerr << "*******EOF " << filename << std::endl;

// 		file.close();
// 		response_complete = true;
//         std::cerr << "all read " << filename << std::endl;
// 	}
// 	position = file.tellg();
// 	// close(_target_fd);
// }

Client::~Client()
{
}