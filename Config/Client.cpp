#include "Client.hpp"

Response &Client::getResp()
{
	return (_resp);
}

bool Client::getQuer()
{
	return (_quer);
}

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

Client::Client(int new_socket, char *clien_ip, t_serv s): _serv(s),
	 _req(_serv.loc), _c(false)
{
	// std::cout << std::endl << std::endl << "MBHERE?" << std::endl << std::endl;
	_clienIP = clien_ip;
	_fdSock = new_socket;
	_serv = s;
	_isClosed = false;
	_isRead = false;
	_toServe = false;
	_errBodySize = 0;
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
		std::cout << "ISCLOSED: " << _isClosed ? "TRUE" : "FALSE";
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
	int err = _req.getErrorCode();
	std::cout << "ERRRRRRORRRR!!!:" << err << std::endl;
	if (err == 301 || err == 200 || err == 0 || err == 201)
		return 1;
	std::string errorPath = "";
	errorPath = checkErrorMap(err);
	std::cout << "Hhhhhhhhhhhhasdfjasfajskf" << errorPath << std::endl;
	if (errorPath != "")
	{
		_req.setResource(errorPath);
		return 1;
	}
	else if (errorPath == "")
	{
		std::cerr << " MB HERE????? \n";
		std::cout << "ERRRRRRORRRR!!!:" << err << std::endl;
		std::string _errPage = genErrPage(err);
		sendErrHTML(_errPage, err); // function to send generated error pages
	}
	return 0;
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
	std::cerr << " MB HERE????? \n";
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

Client::~Client()
{
}