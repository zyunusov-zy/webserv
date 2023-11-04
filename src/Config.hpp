#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "mainIn.hpp"


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
	void confCheck(std::vector<t_serv>& servers);
public:
	Config(/* args */);
	~Config();
	int parse(std::string fileName, std::vector<t_serv>& servers);
	void print1();
};

#endif
