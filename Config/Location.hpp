#ifndef LOCATION_HPP
#define LOCATION_HPP

// #include "Config.hpp"
// #include "mainIn.hpp"
#include <string>
#include <map>
#include <iostream>

struct Location
{
	std::string						path;
	std::string						root;
	std::string						index;
	std::string 					autoindex;
	std::map<std::string , bool>	methods;
	std::pair<int, std::string>		redir;
	std::multimap<std::string, std::string>		cgi;
	int								limit_client_size;

	void initLoc();

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
	std::multimap<std::string, std::string>& getCGI();
	void setBodySize(std::string num);
	int getBodySize() const;
};

#endif