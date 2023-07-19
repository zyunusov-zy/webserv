#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Config.hpp"

class Location
{
private:
	std::string		path;
	std::string		root;
	std::string		index;
	int				limit_client_body_size;

public:
	Location(/* args */);
	~Location();

	void setPath(std::string v);
	std::string getPath();
};

Location::Location(/* args */)
{
	path = "";
	root = "";
	index = "";
	limit_client_body_size = 0;


}

Location::~Location()
{
}

void Location::setPath(std::string v)
{
	if (v == "")
	{
		std::cerr << "Check path of the location\n";
		exit(1);
	}
	this->path = v;
}

std::string Location::getPath()
{
	return this->path;
}



#endif