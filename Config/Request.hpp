#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <map>
#include <cerrno>
#include <cstring>
#include <fstream>
#include "Config.hpp"
#include "Server.hpp"
#include "ErrorCodes.hpp"
#include "Location.hpp"



typedef std::map<std::string, std::string> HeaderMap;

class Config;

class Request 
{
	private:
		std::string _method;
		std::string _uri;
		std::string _version;
		std::string _queryString;
		std::string _body;
		t_serv serv;
		bool _q;
		bool _cgi;

		HeaderMap _headers;
		char *_buf;
		std::string _tmpBuffer;
		int _parseStat;
		int _errorCode;
		std::multimap<std::string, Location> &_locationMap;
		Location	*_location;
		int _bodySize;
		std::string _transEn;
		bool _isChunkSize;
		int _chunkSize;
		bool _isReqDone;
		int _cgiNum;
		char **_envCGI;
		std::string _uriCGI;

		bool doesFileExist(const std::string& filePath);
		void parseResource();
	public:
		Request(std::multimap<std::string, Location> &l);
		~Request();
		bool getQ();
		std::string& getMethod();
		std::string& getResource();
		std::string& getVersion();
		std::string& getBody();
		std::string& getQueryString();
		std::string& getUriCGI();
		int getErrorCode();
		bool getCGIB();
		int getPortOfReq();
		t_serv getServ();
		HeaderMap& getHeaders();
		char** getENV();
		void print();
		

		char	*getBuffer(void) const;
		bool	saveRequestData(ssize_t recv);
		void	resetRequest();
		void	saveStartLineHeaders(std::string &data);
		void	saveStartLine(std::string startLine);
		void	validateStartLine(void);
		Location *getLoc();
		void	parseUri(void);
		void	parsePercent(std::string &s);
		void	saveHeaderLine(std::string headerLine);
		void	saveChunkedBody(std::string &data);
		void	parseChunkSize(std::string &data);
		void	parseChunkedBody(std::string &data);
		void	saveSimpleBody(std::string &data);
		void	setErrorStatus(const int s);
		bool  checkCGI();
		void makeEnv();
		std::string getURI();
		std::string validateURI(std::string &fullPath, std::uint8_t mode);
		void getUriEncodedBody();
};

Request::Request(std::multimap<std::string, Location> &l): _q(false), _errorCode(0), 
_cgi(false), _buf(new char[RECV_BUFFER_SIZE + 1]), 
_parseStat(STARTL), _bodySize(0), _isChunkSize(false),
_chunkSize(0), _isReqDone(false), _locationMap(l), _cgiNum(0), 
_envCGI(NULL)
{
}

bool Request::getQ()
{
	return _q;
}

char	*Request::getBuffer(void) const {
	return _buf;
}

bool Request::getCGIB()
{
	return _cgi;
}


void	Request::setErrorStatus(const int s) {
	_errorCode = s;
}



std::string& Request::getBody()
{
	return _body;
}

std::string& Request::getQueryString()
{
	return _queryString;
}

t_serv Request::getServ()
{
	return serv;
}

char** Request::getENV()
{
	return _envCGI;
}

void Request::resetRequest()
{
	_errorCode = 0;
}

int Request::getErrorCode()
{
	return _errorCode;
}

Location *Request::getLoc()
{
	std::string tmp;
	std::string tmp1;
	size_t lastSlash;
	size_t len;
	bool isLastSlash;

	isLastSlash = false;
	if (_uri[_uri.length() - 1] != '/')
	{
		isLastSlash = true;
		_uri.push_back('/');
	}
	lastSlash = _uri.find_last_of("/");
	if (lastSlash == std::string::npos)
		throw ErrorException(400, "Bad Request");
	tmp = _uri.substr(0, lastSlash);
	len = std::count(_uri.begin(), _uri.end(), '/');
	for(size_t i = 0; i < len; i++)
	{
		std::multimap<std::string, Location>::iterator j = _locationMap.begin();
		for(; j != _locationMap.end(); j++)
		{
			if (!tmp.length())
				tmp = "/";
				(j->first != "/" && j->first[j->first.length() - 1] == '/') ?
					tmp1 = j->first.substr(0, j->first.find_last_of("/")) : tmp1 = j->first;
				if (tmp == tmp1)
				{
					if (isLastSlash)
						_uri.pop_back();
					return &j->second;
				}
		}
		lastSlash = tmp.find_last_of("/", lastSlash);
		tmp = tmp.substr(0, lastSlash);
	}
	return NULL;
}

void	Request::parsePercent(std::string &s)
{
	std::stringstream 	ss;
	std::string 		tmp;
	int					c;

	for (size_t i = 0; i < s.length(); i++)
	{
		if (s[i] == '%')
		{
			try
			{
				ss << std::hex << s.substr(i + 1, 2);
				ss >> c;
				tmp = s.substr(i + 3);
				s.erase(i);
				s.push_back(static_cast<char>(c));
				s.append(tmp);
				ss.clear();

			}
			catch(const std::exception& e)
			{
				throw ErrorException(400, "Bad Request");
			}
			
		}else if (s[i] == '+')
			s = s.substr(0 , i) + " " + s.substr(i + 1);
	}
}

void	Request::parseUri(void)
{
	size_t pos;

	pos = _uri.find("?");
	if (pos != std::string::npos)
	{
		_queryString = _uri.substr(pos + 1);
		_uri.erase(pos);
	}
	parsePercent(_uri);
	if (_queryString.empty())
		_q = false;
}

void	Request::validateStartLine(void)
{
	_location = getLoc();
	if (_location == NULL)
		throw ErrorException(404, "Not Found");
	//need to get location;
	if (_version != "HTTP/1.1")
		throw ErrorException(505, "Http Version Not Supported");
	std::map<std::string, bool>::const_iterator i = _location->methods.begin();
	// std::cout << "hello" << std::endl;
	for(; i != _location->methods.end(); i++)
	{
		if (i->first == _method){
			if (!i->second)
				throw ErrorException(405, "Method Not Allowed");
			break;
		}
	}
	if (i == _location->methods.end())
		throw ErrorException(400, "Bad Request");
	if (_version != "HTTP/1.1")
		throw ErrorException(505, "Http Version Not Supported");
	_bodySize = _location->getBodySize();
	parseUri();
}

void	Request::saveStartLine(std::string startLine)
{
	size_t pos;

	// std::cout << startLine << std::endl << std::endl;


	if (!startLine.length())
		throw ErrorException(400, "Bad Request! No Header of the request!");
	pos = startLine.find(' ');
	if (pos == std::string::npos)
		throw ErrorException(400, "Bad Request!No Separaters in the Head of the request!");
	_method = startLine.substr(0, pos);
	startLine.erase (0, skipWhiteSpaces(startLine, pos));

	pos = startLine.find(' ');
	if (pos == std::string::npos)
		throw ErrorException(400, "Bad Request!No Separaters in the Head of the request!");
	_uri = startLine.substr(0, pos);
	startLine.erase (0, pos);
	_version = startLine;
	_version.erase(std::remove_if(_version.begin(),
		_version.end(), &isCharWhiteSpace), _version.end());


	validateStartLine();
	// std::cout << "heeeeee212121" <<std::endl;
	_parseStat = HEADERL;
	// std::cout << _parseStat << std::endl;
}

void	Request::saveHeaderLine(std::string headerLine)
{
	size_t colPos;
	std::string headerKey;
	std::string headerVal;

	headerLine.erase(std::remove_if(headerLine.begin(),
		headerLine.end(), &isCharWhiteSpace), headerLine.end());
	// std::cout << "hello" << std::endl;
	// std::cout << headerLine << std::endl;
	if (!headerLine.length())
	{
		if (_headers.find("Host") == std::end(_headers))
			throw ErrorException(400, "Bad Request");
		if (_headers.find("Transfer-Encoding") == std::end(_headers)
			&& _headers.find("Content-Length") == std::end(_headers))
			_parseStat = END_STAT;
		else
			_parseStat = BODYL;
		return ;
	}
	colPos = headerLine.find(":");
	if (colPos == std::string::npos)
		throw ErrorException(400, "Bad Request! No separater \":\"");
	headerKey = headerLine.substr(0, colPos);
	headerVal = headerLine.substr(colPos + 1);
	_headers.insert(std::pair<std::string, std::string> (headerKey, headerVal));
	if (headerKey == "Content-Length")
		_bodySize = static_cast<int>(std::atol(headerVal.c_str()));
	if (headerKey == "Transfer-Encoding")
		_transEn = headerVal;
	return ;
}

void	Request::saveStartLineHeaders(std::string &data)
{
	size_t linePos;

	linePos = data.find("\n");
	while(linePos != std::string::npos 
	&& (_parseStat != BODYL && _parseStat != END_STAT))
	{
		if (_parseStat == STARTL)
		{
			saveStartLine(data.substr(0, linePos));
			data.erase(0, linePos + 1);
		}
		if (_parseStat == HEADERL)
		{
			linePos = data.find("\n");
			saveHeaderLine(data.substr(0, linePos));
			data.erase(0, linePos + 1);
		}
		linePos = data.find("\n");
	}
}

void	Request::parseChunkSize(std::string &data)
{
	std::stringstream ss;
	size_t pos;
	
	if (!data.length())
	{
		_parseStat = END_STAT;
		return ;
	}
	pos = data.find("\n");
	if (pos == std::string::npos)
		return ;
	ss << std::hex << data.substr(0, pos);
	ss >> _chunkSize;
	if (!_chunkSize)
		_parseStat = END_STAT;
	_isChunkSize = true;
	data.erase(0, pos + 1);
}

void	Request::parseChunkedBody(std::string &data)
{
	size_t i = 0;

	while(i < data.length() && _chunkSize)
	{
		if (data[i] == '\n' && (i - 1 >= 0 && data[i - 1] == '\r'))
			_body.push_back('\n');
		else if (data[i] != '\r')
			_body.push_back(data[i]);
		i++;
		_chunkSize--;
	}
	if (!_chunkSize)
	{
		_isChunkSize = false;
		i += 2;
	}
	data.erase(0, i);
}

void	Request::saveChunkedBody(std::string &data)
{
	while(_parseStat != END_STAT)
	{
		if (!_isChunkSize)
			parseChunkSize(data);
		if (_isChunkSize && _parseStat != END_STAT)
			parseChunkedBody(data);
	}
}

void	Request::saveSimpleBody(std::string &data)
{
	size_t bodySize;

	bodySize = static_cast<size_t>(std::atol(_headers["Content-Length"].c_str()));
	if (bodySize > this->_bodySize)
		throw ErrorException(413, "Request Entity Too Large");
	if (_body.length() + data.length() > this->_bodySize)
		throw ErrorException(413, "Request Entity Too Large");
	
	_body.append(data);
	data.clear();
	if (_body.length() == bodySize)
		_parseStat = END_STAT;
}

bool	Request::saveRequestData(ssize_t recv)
{
	std::string data;

	data = _tmpBuffer;

	_buf[recv] = '\0';
	data.append(_buf, recv);
	// std::cout << "HERE: " << std::endl << std::endl;
	// std::cout << std::endl << data << std::endl;

	if (_parseStat == END_STAT)
		resetRequest();
	if (_parseStat == STARTL || _parseStat == HEADERL)
		saveStartLineHeaders(data);
	if (_parseStat == BODYL)
	{
		if (_transEn == "chunked")
			saveChunkedBody(data);
		else
			saveSimpleBody(data);
	}
	_tmpBuffer = data;
	if (_parseStat == END_STAT)
		_isReqDone = true;
	return _isReqDone;
}

std::string Request::validateURI(std::string &fullPath, std::uint8_t mode)
{
	std::string tmp;
	if (mode == 2)// dir
	{
		tmp = fullPath + "/" + _location->getIndex();
		if (isDirOrFile(tmp.c_str()) == 1){
			if (!access(tmp.c_str(), R_OK))
				return tmp;
		}
		if (_location->getAutoInd() == "on")
		{
			_errorCode = 1;
			return fullPath;
		}
		if (errno == EACCES)
			_errorCode = 403;
		else
			_errorCode = 404;
		return tmp;
	}
	else if (mode == 1) //file
	{
		if (!access(fullPath.c_str(), R_OK))
			return fullPath;
		if (errno == EACCES)
			_errorCode = 403;
		else
			_errorCode = 404;
		_errorCode = 403;
		return fullPath;
	}
	if (_method == "POST")
		_errorCode = 201;
	else
		_errorCode = 404;
	return fullPath;
}

std::string Request::getURI()
{
	std::string	fullPath;
	std::uint8_t mode;
	std::string tmp;

	if (_location && _location->redir.second.length())
	{
		_errorCode = _location->redir.first;
		return _location->redir.second;
	}
	_errorCode = 200;
	fullPath = _location->getRoot() + _uri;
	for(size_t i = 0; i < fullPath.length() - 1; i++)
	{
		if (fullPath[i] == '/' && fullPath[i + 1] == '/')
			fullPath.erase(i + 1, 1);
	}
	if (fullPath[fullPath.length() - 1] == '/')
		fullPath.pop_back();
	mode = isDirOrFile(fullPath.c_str());
	if (mode == 0 && _method != "POST")
	{
		_errorCode = 404;
		return "unkown url";
	}
	return (validateURI(fullPath, mode));
}

void Request::getUriEncodedBody()
{
	std::string			tmp;
	size_t 				size;
	size_t 				pos;
	std::pair<std::string, std::string> pair;

	tmp = _body;
	size = std::count(tmp.begin(), tmp.end(), '&');
	for(size_t i = 0; i < size; i++)
	{
		pos = tmp.find("&");
		pair.first = tmp.substr(0, tmp.find("="));
		parsePercent(pair.first);
		pair.second = tmp.substr(tmp.find("=") + 1, pos - pair.first.length() - 1);
		parsePercent(pair.second);
		_headers.insert(pair);
		tmp.erase(0 , pos + 1);
	}
}

void Request::makeEnv()
{
	_envCGI = new char*[_headers.size() + 1];

	_envCGI[_headers.size()] = NULL;
	std::map<std::string, std::string>::iterator i = _headers.begin();
	for(int j = 0; i != _headers.end(); i++, j++)
		_envCGI[j] = strdup((i->first + "=" + i->second).c_str());
}

bool  Request::checkCGI()
{
	std::multimap<std::string,std::string> tmp = _location->getCGI();
	std::cout << _location->getRoot() << std::endl;
	std::cout << _location->getCGI().empty() << std::endl;
	std::string ext = "." + _uri.substr(_uri.find_last_of('.') + 1);
	std::cout << "EXTENSION: "  << ext << std::endl;
	std::cout << tmp.empty() << std::endl;
	for(auto v : tmp)
	{
		std::cout << v.first << ": " << v.second << std::endl;
	}
	// if (tmp.empty()) {
    // 	_cgi = false;
    // 	if (access(_uri.c_str(), F_OK) == 0) {
    //     	// The file exists but isn't in the CGI map
    //     	throw ErrorException(500, "Server configuration error: File is present but not configured in CGI map.");
    // 	}
	// }
	std::multimap<std::string,std::string>::const_iterator i = tmp.begin();
	for(; i != tmp.end(); i++)
	{
		if (ext == i->first)
		{
			std::cout <<  "path for script: " <<i->second << "]" << std::endl;
			int n = access(i->second.c_str(), X_OK);
			std::cout << "ACEES: " << n << std::endl;
			if (n == -1)
			{
				std::cerr << "Error: " << strerror(errno) << std::endl;
				_cgiNum = -1;
				break;
			}
			_cgiNum++;
		}
	}
	std::cout << "NUM: "  << _cgiNum << std::endl;
	if (_cgiNum > 0)
	{
		_cgi = true;
		_uriCGI = getURI(); 
		getUriEncodedBody();//need to code
		_headers.insert(std::pair<std::string, std::string>("QUERY_STRING", _queryString));
		_headers.insert(std::pair<std::string, std::string>("REQUEST_METHOD", _method));
		_headers.insert(std::pair<std::string, std::string>("PATH_INFO", _uriCGI));
		_headers.insert(std::pair<std::string, std::string>("PATH_TRANSLATED", _location->getRoot()));
		_headers.insert(std::pair<std::string, std::string>("SERVER_PROTOCOL", "HTTP/1.1"));
		makeEnv();
	}
	else if (_cgiNum < 0){
		_cgi = false;
		throw ErrorException(502, "Bad Gateway");
	}
	else
		_cgi = false;
	std::cout << (_cgi ? "TRUE" : "FALSE") << std::endl;
	return _cgi;
}


void Request::print()
{
	std::cout << "Method: " << getMethod() << "\n";
    std::cout << "Resource: " << getResource() << "\n";
    std::cout << "Version: " << getVersion() << "\n";
	// std::cout << "PATH_INFO: " << getPathInfo() << "\n";
	// std::cout << "ScriptName: " << getScriptName() << "\n";
	std::cout << "QueryString: " << getQueryString() << "\n" << "\n";

	std::cout << "Body: " << getBody() << "\n" << "\n";
	std::cout << "Location Path: " << _location->getPath() << "\n" << "\n";

	HeaderMap tmp = getHeaders();
	std::cout << "Headers: " << getBody() << "\n" << "\n";
	for (auto header : tmp) {
        std::cout << header.first << " = " << header.second << "\n";
    }
}

std::string& Request::getMethod()
{
	return this->_method;
}
std::string& Request::getResource()
{
	return this->_uri;
}
std::string& Request::getVersion()
{
	return this->_version;
}
HeaderMap& Request::getHeaders()
{
	return this->_headers;
}

std::string& Request::getUriCGI()
{
	return _uriCGI;
}

Request::~Request()
{
	// if (_buf)
	// {
	// 	delete [] _buf;
	// 	_buf = nullptr;
	// }
	// if (_envCGI) {
	// 	for (size_t i = 0; i < _headers.size(); ++i) {
	// 		free(_envCGI[i]);
	// 	}
	// 	delete[] _envCGI;
   	// 	_envCGI = nullptr;
	// }
}
#endif