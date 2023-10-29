#include "Location.hpp"
#include "ErrorCodes.hpp"

void Location::initLoc()
{
	path = "";
	root = "";
	index = "";
	autoindex = "of";
	limit_client_size = -1;
	methods["GET"] = true;
	methods["DELETE"] = true;
	methods["POST"] = true;
}


void Location::setPath(std::string v)
{
	if (v == "")
	{
		throw ErrorException(0, "Check path value of the location");
		exit(1);
	}
	this->path = v;
}

std::string Location::getPath() const
{
	return this->path;
}

void Location::setBodySize(std::string num)
{
	std::istringstream iss(num);
    long long value;

    if (!(iss >> value) || value <= 0 || value > std::numeric_limits<int>::max()) {
        throw ErrorException(0, "Invalid or Negative Body Size or Overflow");
    }

    limit_client_size = static_cast<int>(value);

}
int  Location::getBodySize() const
{
	return limit_client_size;
}

void Location::setAutoInd(std::string v)
{
	if (v == "")
	{
		ErrorException(0, "Check autoindex value of the location");
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
        throw ErrorException(0, "Check root value there is not root value");
	}
	if (v[v.length() - 1] != '/')
		v.push_back('/');
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
		throw ErrorException(0, "Check index value of the location");
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
		throw ErrorException(0, "Check return value of the location");
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
		throw ErrorException(0, "Check cgi_path value of the location");
	}
	if (v[v.length() - 1] != '/')
		v.push_back('/');
	this->cgi.insert(std::pair<std::string, std::string>(k, v));
}

std::multimap<std::string, std::string>& Location::getCGI()
{
	return this->cgi;
}

void Location::mapingMethods(std::string &str)
{
	if (str.find("GET") == std::string::npos)
	{
		methods["GET"] = false;
	}
	else if (str.find("POST") == std::string::npos)
	{
		methods["POST"] = false;
	}
	else if (str.find("DELETE") == std::string::npos)
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