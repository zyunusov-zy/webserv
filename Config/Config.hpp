#ifndef CONFIG_HPP
# define CONFIG_HPP

// #include <fstream>
// #include <cstdlib>
// #include <cstring>
// #include <vector>
// #include <algorithm>
// #include <map>
// #include "Location.hpp"
#include "mainIn.hpp"
// #include "ErrorCodes.hpp"
// #include "Request.hpp"
// #include "ErrorCodes.hpp"

# define RED "\033[31m"
# define NORMAL "\033[0m"



class Config
{
private:
	std::vector<std::string> configRead(std::string fileName);
	t_serv parseTokens(size_t& i, std::vector<std::string>& tokens);
	size_t findNth(const std::vector<std::string> s, size_t i);
	void initServer(t_serv &t);
	void	mapingErrorPage(t_serv& t, std::string &value, std::string& key);
	void valueForServer(std::vector<std::string> tokens, size_t end, size_t& start, t_serv& t);
	void parsLocation(std::vector<std::string> tokens, size_t end, size_t start, t_serv& server);
	int findLoc(size_t i, std::vector<std::string> tokens);
	void checkPathCGI(std::vector<t_serv>& servers);
	void modifyCGIMap(const std::string& root, std::multimap<std::string, std::string>& cgiMap);

	// std::vector<t_serv> servers;
	// std::map<int, std::string> errorPage;
	// friend class Server;
	// friend class Request;
public:
	Config(/* args */);
	~Config();
	int parse(std::string fileName, std::vector<t_serv>& servers);
};

#endif