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
		std::string _requestline;
		std::string _method;
		std::string _uri;
		std::string _version;
		std::string	_scriptName;
		std::string _pathInfo;
		std::string _queryString;
		int			_portOfReq;
		// HeaderMap _query;
		std::string _body;
		t_serv serv;
		// int _err;
		bool _q;
		bool _cgi;

		HeaderMap _headers;
		char *_buf;
		std::string _tmpBuffer;
		int _parseStat;
		int _errorCode;
		// const Location *_location;
		int _bodySize;
		std::string _transEn;
		bool _isChunkSize;
		int _chunkSize;
		bool _isReqDone;

		bool doesFileExist(const std::string& filePath);
		void parseResource();
	public:
		Request();
		~Request();
		bool getQ();
		std::string& getMethod();
		std::string& getResource();
		std::string& getVersion();
		std::string& getScriptName();
		std::string& getPathInfo();
		std::string& getBody();
		std::string& getQueryString();
		int getErrorCode();
		bool getCGIB();
		int getPortOfReq();
		t_serv getServ();
		HeaderMap& getHeaders();
		void print();
		

		char	*getBuffer(void) const;
		bool	saveRequestData(ssize_t recv);
		void	resetRequest();
		void	saveStartLineHeaders(std::string &data);
		void	saveStartLine(std::string startLine);
		void	validateStartLine(void);
		// const Location *getLoc();
		void	parseUri(void);
		void	parsePercent();
		void	saveHeaderLine(std::string headerLine);
		void	saveChunkedBody(std::string &data);
		void	parseChunkSize(std::string &data);
		void	parseChunkedBody(std::string &data);
		void	saveSimpleBody(std::string &data);
		void	setErrorStatus(const int s);
};

Request::Request(): _q(false), _errorCode(0), 
_cgi(false), _buf(new char[RECV_BUFFER_SIZE + 1]), 
_parseStat(STARTL), _bodySize(0), _isChunkSize(false),
_chunkSize(0), _isReqDone(false)
{
}

bool Request::getQ()
{
	return _q;
}

int Request::getPortOfReq()
{
	return _portOfReq;
}

std::string& Request::getScriptName()
{
	return _scriptName;
}

char	*Request::getBuffer(void) const {
	return _buf;
}

bool Request::getCGIB()
{
	return _cgi;
}

std::string& Request::getPathInfo()
{
	return _pathInfo;
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

void Request::resetRequest()
{
	_errorCode = 0;
}

int Request::getErrorCode()
{
	return _errorCode;
}

// const Location *Request::getLoc()
// {
// 	std::string tmp;
// 	std::string tmp1;
// 	size_t lastSlash;
// 	size_t len;
// 	bool isLastSlash;

// 	isLastSlash = false;
// 	if (_uri[_uri.length() - 1] != '/')
// 	{
// 		isLastSlash = true;
// 		_uri.push_back('/');
// 	}
// 	lastSlash = _uri.find_last_of("/");
// 	if (lastSlash == std::string::npos)
// 		throw ErrorException(400, "Bad Request");
// 	tmp = _uri.substr(0, lastSlash);
// 	len = std::cout(_uri.begin(), _uri.end(), '/');
// 	for(size_t i = 0; i < len; i++)
// 	{

// 	}
// }

void	Request::parsePercent()
{
	std::stringstream 	ss;
	std::string 		tmp;
	int					c;

	for (size_t i = 0; i < _uri.length(); i++)
	{
		if (_uri[i] == '%')
		{
			try
			{
				ss << std::hex << _uri.substr(i + 1, 2);
				ss >> c;
				tmp = _uri.substr(i + 3);
				_uri.erase(i);
				_uri.push_back(static_cast<char>(c));
				_uri.append(tmp);
				ss.clear();

			}
			catch(const std::exception& e)
			{
				throw ErrorException(400, "Bad Request");
			}
			
		}else if (_uri[i] == '+')
			_uri = _uri.substr(0 , i) + " " + _uri.substr(i + 1);
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
	parsePercent();
	if (_queryString.empty())
		_q = false;
}

void	Request::validateStartLine(void)
{
	// _location = getLoc();
	//need to get location;
	if (_version != "HTTP/1.1")
		throw ErrorException(505, "Http Version Not Supported");
	// _bodySize = _location.getBodySize();
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

// bool Request::doesFileExist(const std::string& filePath) {
//     std::ifstream infile(filePath.c_str());
//     return infile.good();
// }

// void Request::parseResource()
// {
// 	std::string scriptDir = "/cgi-bin";
// 	size_t pos = _uri.find(scriptDir);

// 	if (pos != std::string::npos) {
// 		// Handle as a CGI request
// 		pos += scriptDir.length();
// 		std::string remainingPath = _resource.substr(pos);
// 		std::istringstream ss(remainingPath);
// 		std::string segment;
// 		while (std::getline(ss, segment, '/')) {
// 			if (doesFileExist(scriptDir + '/' + segment)) {
// 				_scriptName = scriptDir + '/' + segment;
// 				// Find the position of scriptName in resource to split the pathInfo
// 				pos = _resource.find(_scriptName);
// 				pos += _scriptName.length();
// 				if (pos < _resource.length()) {
// 					_pathInfo = _resource.substr(pos);
// 				} else {
// 					_pathInfo = "";
// 				}
// 				return;
// 			}
// 		}
// 		_cgi = true;
// 		throw std::runtime_error("Invalid resource path: " + _resource);
// 	} else {
// 		// Non-CGI request, treat as a file to serve
// 		_scriptName = _resource;
// 		_pathInfo = _resource;
// 	}
// }



void Request::print()
{
	std::cout << "Method: " << getMethod() << "\n";
    std::cout << "Resource: " << getResource() << "\n";
    std::cout << "Version: " << getVersion() << "\n";
	// std::cout << "PATH_INFO: " << getPathInfo() << "\n";
	// std::cout << "ScriptName: " << getScriptName() << "\n";
	std::cout << "QueryString: " << getQueryString() << "\n" << "\n";

	std::cout << "Body: " << getBody() << "\n" << "\n";


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

Request::~Request()
{
}
#endif