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

Client::Client(int new_socket, char *clien_ip, t_serv s):
	 _req(_serv.loc), _c(false), _serv(s)
{
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
	}
	catch(ErrorException &e)
	{
		_toServe = true;
		std::cout << e.what() << std::endl;
		_req.setErrorStatus(e.getStatus());
		throw ErrorException(_req.getErrorCode(), "Error 413\n");
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
			if (it->second.find(_serv.Mroot) == std::string::npos)
			{
				it->second = _serv.Mroot + it->second;
			}
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

	std::cout << "IN CHECK ERROR" << std::endl;



	if (getResp()->exec_err_code > 0)
	{
		err = getResp()->exec_err_code;
		getResp()->exec_err_code = 0;
	}
	else
		err = _req.getErrorCode();

	std::cout << err << std::endl;

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
			sendErrHTML(indexAuto, err);
			return 0;
		}
	}
	std::string errorPath = "";
	errorPath = checkErrorMap(err);
	if (errorPath != "")
	{
		_req.setResource(errorPath);
		return 1;
	}
	else if (errorPath == "")
	{
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
	pos = path.rfind(loc->root);
	if (fileName == "." || fileName == ".." || pos == std::string::npos)
	{
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
		errMap.insert(std::pair<int, std::string>(504, " Gateway Timeout"));

	}
	return errMap[err];
}

std::string	Client::genErrPage(int err)
{
	std::stringstream buff;
	buff <<  "<html>\n";
	buff <<  "<head><title>" << err << errorMap(err) << "</title></head>\n";
	buff <<  "<body>\n";
	buff <<	"<div><h1>" << err << errorMap(err) << "</h1>\n";
	buff << "</body>\n";
	buff <<  "</html>\n";

	std::string res = buff.str();
	return res;
}

void Client::print()
{
	_req.print();
	std::cout << "Error_code: " << _req.getErrorCode() << std::endl;
	std::cout << "Query: " << _req.getQ() << std::endl;
	std::cout << "Server host: " << _serv.host << std::endl;
}

Client::~Client()
{
	delete[] _req.getBuffer();
}
