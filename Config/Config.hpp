#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <map>
#include "Location.hpp"

# define RED "\033[31m"
# define NORMAL "\033[0m"

class Server;
class Request;

typedef struct s_serv
{
	std::string								ipPort;
	std::string								namePort;
	std::string								name;
	std::string 							host;
	int										port;
	std::map<int, std::string>				errorPages;
	int										limit_client_size;
	std::map<std::string, Location>			loc;
}	t_serv;

class Config
{
private:
	std::vector<std::string> configRead(std::string fileName);
	t_serv parseTokens(size_t& i, std::vector<std::string>& tokens);
	size_t findNth(const std::vector<std::string> s, size_t i);
	void initServer(t_serv &t);
	void trim(std::string& s, char c);
	void	mapingErrorPage(t_serv& t, std::string &value, std::string& key);
	void valueForServer(std::vector<std::string> tokens, size_t end, size_t& start, t_serv& t);
	void parsLocation(std::vector<std::string> tokens, size_t end, size_t start, t_serv& server);
	int findLoc(size_t i, std::vector<std::string> tokens);

	std::vector<t_serv> servers;
	// std::map<int, std::string> errorPage;
	friend class Server;
	friend class Request;
public:
	Config(/* args */);
	~Config();
	int parse(std::string fileName);
};

Config::Config(/* args */)
{
}

Config::~Config()
{

	// for(int i = 0; i < servers.size(); i++)
	// {
	// 	servers[i].errorPages.clear();
	// 	servers[i].loc.clear();
	// }
	// servers.clear();
}


std::vector<std::string> Config::configRead(std::string fileName)
{
	std::ifstream file(fileName.c_str());
	std::string buf;
	std::string cbuf;

	if (!file.is_open()) {
		std::cerr << "Config file not found\n";
		exit(1);
	}
	while (std::getline(file, buf)) {
		cbuf += buf + '\n';
	}

	char* tmp = new char[cbuf.length() + 1];
	std::strcpy(tmp, cbuf.c_str());

	std::vector<std::string> tokens;
	char* token = std::strtok(tmp, "\n\t");
	while (token != NULL) {
		std::string l = token;
		tokens.push_back(l);
		// delete[] token;
		token = std::strtok(NULL, "\n\t");
	}
	for (size_t i = 0; i < tokens.size(); i++) {
		size_t pos = tokens[i].find_first_not_of(' ');
		if (pos != std::string::npos){
			tokens[i].erase(0, pos);
		}
		if (tokens[i].find("server") != std::string::npos)
			trim(tokens[i], ' ');
	}
	delete[] tmp;
	return (tokens);
}

void Config::trim(std::string& s, char c)
{
	for(size_t j = 0; j < s.size(); j++)
	{
		if (s[j] == c) {
    		s.erase(j, 1);
			--j;
       	}
	}
}

void Config::initServer(t_serv &t)
{
	t.name = "";
	t.host = "";
	t.port = 0;
	// t.logFile = "";
	t.limit_client_size = -1;
}

size_t Config::findNth(const std::vector<std::string> s, size_t i)
{
	for(; i < s.size(); i++)
	{
		if (s[i] == "server")
		{
			// std::cout << s[i] << std::endl;
			return (i - 1);
		}
	}
	return i - 1;
}

void	Config::mapingErrorPage(t_serv& t, std::string &value, std::string& str) {
	t.errorPages.insert(std::pair<int, std::string>(atoi(value.c_str()), str));
}

void Config::valueForServer(std::vector<std::string> tokens, size_t end, size_t& start, t_serv& t)
{
	std::string p = "";
	for(; start < end; start++)
	{
		if (tokens[start].find("host") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("host") + strlen("host"));
			trim(v, ' ');
			trim(v, ';');
			t.host = v;
			// std::cout << t.host << std::endl << std::endl;
		}
		if (tokens[start].find("server_name") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("server_name") + strlen("server_name"));
			trim(v, ' ');
			trim(v, ';');
			t.name = v;
		}
		if (tokens[start].find("limits_client_body_size") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("limits_client_body_size") + strlen("limits_client_body_size"));
			trim(v, ' ');
			trim(v, ';');
			t.limit_client_size = atoi(v.c_str());
		}
		if (tokens[start].find("listen") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("listen") + strlen("listen"));
			trim(v, ' ');
			trim(v, ';');
			p = v;
			t.port = atoi(v.c_str());
		}
		if (tokens[start].find("error_page") != std::string::npos)
		{
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			std::string key = tokens[start].substr(0, tokens[start].find(' '));
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			trim(tokens[start], ' ');
			std::string subStr = tokens[start].substr(0, tokens[start].find(';'));
			mapingErrorPage(t,key, subStr);
		}
		if (tokens[start].find("location") != std::string::npos)
			break;
	}
	if (p != "" && t.host != "")
	{
		std::string tmp = t.host + ":" + p;
		t.ipPort = tmp;
	}
	if (p != "" && t.name != "")
	{
		std::string tmp1 = t.name + ":" + p;
		t.namePort = tmp1;
	}
}

void Config::parsLocation(std::vector<std::string> tokens, size_t end, size_t start, t_serv& server)
{
	// std::cout << start << " = " << tokens[start] << " " << end << std::endl;
	Location l;

	for(; start < end; start++)
	{
		// std::cout << tokens[start] << std::endl;
		if (tokens[start].find("location") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("location") + strlen("location"));
			trim(v, ' ');
			// std::cout <<  v << std::endl << std::endl;
			l.setPath(v);
		}
		if (tokens[start].find("root") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("root") + strlen("root"));
			trim(v, ' ');
			trim(v, ';');
			// std::cout <<  v << std::endl << std::endl;
			l.setRoot(v);
		}
		if (tokens[start].find("autoindex") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("autoindex") + strlen("autoindex"));
			trim(v, ' ');
			trim(v, ';');
			// std::cout <<  v << std::endl << std::endl;
			l.setAutoInd(v);
		}
		if (tokens[start].find("index.html") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("index") + strlen("index"));
			trim(v, ' ');
			trim(v, ';');
			// std::cout <<  v << std::endl << std::endl;
			l.setIndex(v);
		}
		if (tokens[start].find("return") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("return") + strlen("return"));
			trim(v, ' ');
			trim(v, ';');
			l.setRedir(v);

		}
		if (tokens[start].find("allow_methods") != std::string::npos)
		{
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			l.mapingMethods(tokens[start]);
		}
		if (tokens[start].find("cgi_path") != std::string::npos)
		{
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			while(tokens[start].length() != 0)
			{
				std::string subStr = tokens[start].substr(0, tokens[start].find(' '));
				if (tokens[start].find(' ') == std::string::npos)
				{
					// std::cout << "HERE" <<std::endl;
					subStr = tokens[start].substr(0, tokens[start].length());
					tokens[start].erase(0, tokens[start].length());
					tokens[start] = "";
				}
				tokens[start].erase(0, tokens[start].find(' ') + 1);
				trim(tokens[start], ';');
				l.setCGI(subStr);
			}
		}
	}
	// std::cout << l.getPath() << std::endl;
	if (l.getPath() != "")
	{
		// std::cout << " helloooo " << std::endl;
		server.loc.insert(std::pair<std::string, Location>(l.getPath(), l));
	}

	// exit(0);
}

int Config::findLoc(size_t i, std::vector<std::string> tokens)
{
	i++;
	for(; i < tokens.size(); ++i)
	{
		if (tokens[i].find("location") < tokens.size())
			return i - 1;
	}
	return i - 1;
}

t_serv Config::parseTokens(size_t& i, std::vector<std::string>& tokens)
{
	t_serv server;
	initServer(server);

	// std::string blockServ;
	size_t pos;
	size_t locEnd;
	if ((pos = findNth(tokens, i)))
	{
		valueForServer(tokens, pos, i, server);
		for(; i < pos; i++)
		{
			if ((locEnd = findLoc(i, tokens)) != -1)
			{
				// std::cout << i << "      " << locEnd << std::endl << std::endl;
				parsLocation(tokens, locEnd, i, server);
				i = locEnd;
			}
		}
		i = pos;
	}
	return server; // 
}

int Config::parse(std::string fileName)
{
	std::vector<std::string> tokens = configRead(fileName);
	// for(size_t i = 0; i < tokens.size(); i++)
	// {
	// 	std::cout << tokens[i] << std::endl;
	// }
	// exit(1);
	for(size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i] == "server")
		{
			++i;
			if (tokens[i] != "{")
			{
				std::cerr << RED << "Error: expected '{' after server directive." << NORMAL << std::endl;
				exit(1);
			}
			++i;
			t_serv server = parseTokens(i, tokens);
			// std::cout << tokens[i] << i << std::endl;
			if (tokens[i] != "}")
			{
				std::cerr << RED << "Error: expected '}' after server directive." << NORMAL << std::endl;
				exit(1);
			}
			this->servers.push_back(server);
			// std::cout << "hello";
			// exit(1);
		}
		else
		{
			std::cerr << RED << "Error: unknown directive [" << tokens[i] << "]" << NORMAL << std::endl;
			exit(1);
		}

	}

	// for(size_t i = 0; i < servers.size(); i++)
	// {
	// 	std::cout << "host:" << servers[i].host << std::endl;
	// 	std::cout << "name:" << servers[i].name << std::endl;
	// 	std::cout << "port:" << servers[i].port << std::endl;
	// 	std::cout << "body_size:" << servers[i].limit_client_size << std::endl;
	// 	std::cout << "MAP: \n";
	// 	for(auto t : servers[i].errorPages)
	// 	{
	// 		std::cout << t.first << ": " << t.second << std::endl;
	// 	}
	// 	std::cout << std::endl << std::endl<< "MAP of loc: \n";
	// 	for(auto t : servers[i].loc)
	// 	{
	// 		std::cout << t.first << ": " << t.second.getPath() << std::endl;
	// 		std::cout << t.second.getPath() << std::endl << std::endl;
	// 		std::cout << t.second.getIndex() << std::endl << std::endl;
	// 		std::cout << t.second.getAutoInd() << std::endl;
	// 		std::cout << t.second.getRoot() << std::endl;
	// 		std::map<std::string , bool> tmp = t.second.getMethods();
	// 		std::cout << "Methods: " << std::endl;
	// 		for( auto s : tmp)
	// 		{
	// 			std::cout << s.first << ": " << s.second << std::endl;
	// 		}

	// 		std::cout << t.second.getRedir() << std::endl << std::endl;
	// 		std::cout << "CGI_PATH: " << std::endl;
	// 		std::vector<std::string> s = t.second.getCGI();
	// 		for(auto c : s)
	// 		{
	// 			std::cout << c << std::endl;
	// 		}
	// 		std::cout <<  std::endl << std::endl;
	// 	}
	// }
	// // tokens.clear();
	return 0;
}

#endif