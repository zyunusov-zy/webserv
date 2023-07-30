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
	std::string						redir;
	std::vector<std::string>		cgi;

public:
	Location(/* args */);
	~Location();

	void setPath(std::string v);
	std::string getPath();
	std::string getAutoInd();
	void setAutoInd(std::string v);
	void setRoot(std::string v);
	std::string getRoot();
	void setIndex(std::string v);
	std::string getIndex();
	void mapingMethods(std::string &s);
	int checkLoc();
	std::map<std::string , bool> getMethods();
	void setRedir(std::string v);
	std::string getRedir();
	void setCGI(std::string v);
	std::vector<std::string> getCGI();
};

Location::Location(/* args */)
{
	path = "";
	root = "";
	index = "";
	redir = "";
	autoindex = "off";
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

std::string Location::getPath()
{
	return this->path;
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

std::string Location::getAutoInd()
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

std::string Location::getRoot()
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

std::string Location::getIndex()
{
	return this->index;
}

void Location::setRedir(std::string v)
{
	if (v == "")
	{
		std::cerr << "Check return value of the location:" << this->path << std::endl;
		exit(1);
	}
	this->redir = v;
}

std::string Location::getRedir()
{
	return this->redir;
}

void Location::setCGI(std::string v)
{
	if (v == "")
	{
		std::cerr << "Check cgo_path value of the location:" << this->path << std::endl;
		exit(1);
	}
	this->cgi.push_back(v);
}

std::vector<std::string> Location::getCGI()
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

std::map<std::string , bool> Location::getMethods()
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