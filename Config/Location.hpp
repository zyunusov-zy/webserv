#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Config.hpp"
#include <iostream>
#include <map>

class Location
{
private:
	std::string						path;
	std::string						root;
	std::string						index;
	std::string 					autoindex;
	std::map<std::string , bool>	methods;
	std::pair<int, std::string>		redir;
	std::multimap<std::string, std::string>		cgi;
	int								limit_client_size;
	friend class Request;
public:
	Location(/* args */);
	~Location();

	void setPath(std::string v);
	std::string getPath() const;
	std::string getAutoInd() const;
	void setAutoInd(std::string v);
	void setRoot(std::string v);
	std::string getRoot() const;
	void setIndex(std::string v);
	std::string getIndex() const;
	void mapingMethods(std::string &s);
	int checkLoc();
	std::map<std::string , bool> getMethods() const;
	void setRedir(std::string v, std::string p);
	const std::pair<int, std::string>& getRedir() const;
	void setCGI(std::string k, std::string v);
	std::multimap<std::string, std::string> getCGI() const;
	void setBodySize(int num);
	int getBodySize() const;
};

Location::Location(/* args */)
{
	path = "";
	root = "";
	index = "";
	autoindex = "off";
	limit_client_size = -1;
	methods["GET"] = true;
	methods["DELETE"] = true;
	methods["POST"] = true;
}

Location::~Location()
{
	this->cgi.clear();
}

void Location::setPath(std::string v)
{
	if (v == "")
	{
		std::cerr << "Check path value of the location:" << this->path << std::endl;
		exit(1);
	}
	this->path = v;
}

std::string Location::getPath() const
{
	return this->path;
}

void Location::setBodySize(int num)
{
	if (num <= 0)
	{
		std::cerr << "Body size cant be 0 or negative numbers of the location:" << this->path << std::endl;
		exit(1);
	}
	limit_client_size = num;
}
int  Location::getBodySize() const
{
	return limit_client_size;
}

void Location::setAutoInd(std::string v)
{
	if (v == "")
	{
		std::cerr << "Check autoindex value of the location:" << this->path << std::endl;
		exit(1);
	}
	this->autoindex = v;
}

std::string Location::getAutoInd() const
{
	return this->autoindex;
}

void Location::setRoot(std::string v)
{
	if (v == "")
	{
		std::cerr << "Check root value of the location:" << this->path << std::endl;
		exit(1);
	}
	this->root = v;
}

std::string Location::getRoot() const
{
	return this->root;
}

void Location::setIndex(std::string v)
{
	if (v == "")
	{
		std::cerr << "Check index value of the location:" << this->path << std::endl;
		exit(1);
	}
	if (this->index != "")
	{
		std::cout << "already has a value\n";
		return ;
	}
	this->index = v;
}

std::string Location::getIndex() const
{
	return this->index;
}

void Location::setRedir(std::string v, std::string p)
{
	if (v == "")
	{
		std::cerr << "Check return value of the location:" << this->path << std::endl;
		exit(1);
	}
	this->redir = std::pair<int, std::string>(atoi(v.c_str()), p);
}

const std::pair<int, std::string>& Location::getRedir() const
{
	return this->redir;
}

void Location::setCGI(std::string k, std::string v)
{
	if (v == "")
	{
		std::cerr << "Check cgi_path value of the location:" << this->path << std::endl;
		exit(1);
	}
	this->cgi.insert(std::pair<std::string, std::string>(k, v));
}

std::multimap<std::string, std::string> Location::getCGI() const
{
	return this->cgi;
}

void Location::mapingMethods(std::string &str)
{
	if (str.find("GET") == std::string::npos)
	{
		methods["GET"] = false;
	}
	if (str.find("POST") == std::string::npos)
	{
		methods["POST"] = false;
	}
	if (str.find("DELETE") == std::string::npos)
	{
		methods["DELETE"] = false;
	}
}

std::map<std::string , bool> Location::getMethods() const
{
	return this->methods;
}

int Location::checkLoc()
{
	// if (this->autoindex != "off" || this->autoindex != "on")
	// {
	// 	std::cout << " helloooo " << std::endl;
	// 	return 0;
	// }
	if (this->path == "")
		return 0;
	return 1;
}

#endif