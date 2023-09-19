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
		return 0;
	std::string errorPath = "";
	errorPath = checkErrorMap(err);
	std::cout << "Hhhhhhhhhhhhasdfjasfajskf" << errorPath << std::endl;
	if (errorPath != "")
		_req.setResource(errorPath);
	// if (errorPath == "")
		// need to write Error page generate function
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