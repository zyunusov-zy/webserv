#include "Config.hpp"

Config::Config(/* args */)
{
}

Config::~Config()
{
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


void Config::initServer(t_serv &t)
{
	t.name = "";
	t.host = "";
}

size_t Config::findNth(const std::vector<std::string> s, size_t i)
{
	for(; i < s.size(); i++)
	{
		if (s[i] == "server")
		{
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
		if (tokens[start].find("host:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("host:") + strlen("host:"));
			trim(v, ' ');
			trim(v, ';');
			t.host = v;
		}
		if (tokens[start].find("server_name:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("server_name:") + strlen("server_name:"));
			trim(v, ' ');
			trim(v, ';');
			t.name = v;
			
		}
		if (tokens[start].find("listen:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("listen:") + strlen("listen:"));
			trim(v, ' ');
			trim(v, ';');
			p = v;
			t.port.push_back(atoi(v.c_str()));
		}
		if (tokens[start].find("error_page:") != std::string::npos)
		{
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			std::string key = tokens[start].substr(0, tokens[start].find(' '));
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			trim(tokens[start], ' ');
			std::string subStr = tokens[start].substr(0, tokens[start].find(';'));
			mapingErrorPage(t,key, subStr);
		}
		if (tokens[start].find("root:") != std::string::npos)
		{
			if (t.Mroot != "")
				throw ErrorException(0, "Cannot be two or more roots");
			std::string v = tokens[start].substr(tokens[start].find("root:") + strlen("root:"));
			trim(v, ' ');
			trim(v, ';');
			t.Mroot = v;
		}
		if (tokens[start].find("location:") != std::string::npos)
			break;
	}
}

void Config::parsLocation(std::vector<std::string> tokens, size_t end, size_t start, t_serv& server)
{
	Location l;
	l.initLoc();

	for(; start < end; start++)
	{
		if (tokens[start].find("location:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("location:") + strlen("location:"));
			trim(v, ' ');
			l.setPath(v);
			l.setRoot(server.Mroot);
		}
		if (tokens[start].find("autoi:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("autoi:") + strlen("autoi:"));
			trim(v, ' ');
			trim(v, ';');
			l.setAutoInd(v);
		}
		if (tokens[start].find("index:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("index:") + strlen("index:"));
			trim(v, ' ');
			trim(v, ';');
			l.setIndex(v);
		}
		if (tokens[start].find("return:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("return:") + strlen("return:"));
			trimBegin(v);
			trim(v, ';');
			std::string extn = v.substr(0, v.find(' '));
			std::string path = v.substr(v.find(extn) + extn.length());
			trim(path, ' ');
			l.setRedir(extn, path);

		}
		if (tokens[start].find("limits_client_body_size:") != std::string::npos)
		{
			std::string v = tokens[start].substr(tokens[start].find("limits_client_body_size:") + strlen("limits_client_body_size:"));
			trim(v, ' ');
			trim(v, ';');
			l.setBodySize(v);
		}
		if (tokens[start].find("allow_methods:") != std::string::npos)
		{
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			l.mapingMethods(tokens[start]);
		}
		if (tokens[start].find("cgi_path:") != std::string::npos)
		{
			tokens[start].erase(0, tokens[start].find(' ') + 1);
			trim(tokens[start], ';');
			while(!tokens[start].empty())
			{
				std::string key = tokens[start].substr(0, tokens[start].find('='));
				trim(key, ' ');
				trim(key, '\t');
				tokens[start].erase(0, tokens[start].find('=') + 1);
				std::string value;
				if (tokens[start].find(' ') != std::string::npos)
				{
					value = tokens[start].substr(0, tokens[start].find(' '));
					tokens[start].erase(0, tokens[start].find(' ') + 1);
				}else{
					value = tokens[start];
					tokens[start].clear();
				}
				trim(value, ' ');
				trim(value, '\t');
				l.setCGI(key, value);
			}
		}
	}
	if (l.getPath() != "")
	{
		server.loc.insert(std::pair<std::string, Location>(l.getPath(), l));
	}

}

int Config::findLoc(size_t i, std::vector<std::string> tokens)
{
	i++;
	for(; i < tokens.size(); ++i)
	{
		if (tokens[i].find("location:") < tokens.size())
			return i - 1;
	}
	return i - 1;
}

t_serv Config::parseTokens(size_t& i, std::vector<std::string>& tokens)
{
	t_serv server;
	initServer(server);

	size_t pos;
	size_t locEnd;
	if ((pos = findNth(tokens, i)))
	{
		valueForServer(tokens, pos, i, server);
		for(; i < pos; i++)
		{
			if ((locEnd = findLoc(i, tokens)) != (size_t)-1)
			{
				parsLocation(tokens, locEnd, i, server);
				i = locEnd;
			}
		}
		i = pos;
	}
	return server; // 
}

void Config::modifyCGIMap(const std::string& root, std::multimap<std::string, std::string>& cgiMap)
{
    for(std::multimap<std::string, std::string>::iterator it = cgiMap.begin(); it != cgiMap.end(); ++it)
    {
        if (!it->second.empty() && (it->second[0] != '/' && root[root.length() - 1] !='/'))
        {
            it->second.insert(0, "/");
        }
        if (it->second.find(root) == std::string::npos)
        {
            it->second.insert(0, root); // prepend the root
        }
        if (it->second.find("//") != std::string::npos)
        {
            it->second.replace(it->second.find("//"), 2, "/");
        }
    }
}


void Config::checkPathCGI(std::vector<t_serv>& servers)
{
	for(size_t i = 0; i < servers.size(); i++)
	{
		for(std::multimap<std::string, Location>::iterator v = servers[i].loc.begin(); v != servers[i].loc.end(); v++)
		{
			if (servers[i].Mroot.empty())
				continue;

			std::multimap<std::string, std::string>& cgiMap = v->second.getCGI();
			if (cgiMap.empty())
				continue;
			modifyCGIMap(servers[i].Mroot, cgiMap);
		}
	}
}

void Config::confCheck(std::vector<t_serv>& servers)
{
    std::set<int> seenPorts;  // Store unique port values

    for (size_t i = 0; i < servers.size(); i++)
    {
        if (servers[i].host == "")
            throw ErrorException(0, "There is no IP (host) on this server");

        // Check for duplicate ports within the server's port vector
        std::set<int> serverPorts;
        for (size_t j = 0; j < servers[i].port.size(); j++)
        {
            int port = servers[i].port[j];
            if (serverPorts.count(port) > 0)
                throw ErrorException(0, "Duplicate port found in the configuration");
            serverPorts.insert(port);

            // Check if the port is already seen in the global context
            if (seenPorts.count(port) > 0)
                throw ErrorException(0, "Duplicate port found in the configuration");
            seenPorts.insert(port);
        }

        // Your other checks...
        if (servers[i].Mroot == "")
            throw ErrorException(0, "There is no root in the server");
        for (std::multimap<std::string, Location>::iterator v = servers[i].loc.begin(); v != servers[i].loc.end(); ++v)
        {
            if (v->second.getPath() == "")
            {
                throw ErrorException(0, "No path in one of the locations");
            }
        }
    }
}


int Config::parse(std::string fileName, std::vector<t_serv>& servers)
{
	std::vector<std::string> tokens = configRead(fileName);
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
			if (tokens[i] != "}")
			{
				std::cerr << RED << "Error: expected '}' after server directive." << NORMAL << std::endl;
				exit(1);
			}
			servers.push_back(server);
		}
		else
		{
			std::cerr << RED << "Error: unknown directive [" << tokens[i] << "]" << NORMAL << std::endl;
			exit(1);
		}
	
	}
	checkPathCGI(servers);

	try{
		confCheck(servers);
	}
	catch(ErrorException &e)
    {
		throw ErrorException(0, e.what());
    }

	return 0;
}

void Config::print1()
{
		for (size_t i = 0; i < servers.size(); i++) {
		std::cout << "host:" << servers[i].host << std::endl;
		std::cout << "name:" << servers[i].name << std::endl;

		for (size_t j = 0; j < servers[i].port.size(); j++) {
			std::cout << "port:" << servers[i].port[j] << std::endl;
		}

		std::cout << "MAP: \n";
		for (std::map<int, std::string>::iterator it = servers[i].errorPages.begin(); it != servers[i].errorPages.end(); ++it) {
			std::cout << it->first << ": " << it->second << std::endl;
		}

		std::cout << std::endl << std::endl << "MAP of loc: \n";
		for (std::multimap<std::string, Location>::iterator it = servers[i].loc.begin(); it != servers[i].loc.end(); ++it) {
			std::cout << it->first << ": " << it->second.getPath() << std::endl;
			std::cout << it->second.getPath() << std::endl << std::endl;
			std::cout << it->second.getIndex() << std::endl << std::endl;
			std::cout << it->second.getAutoInd() << std::endl;
			std::cout << it->second.getRoot() << std::endl;
			std::cout << "Body_size: " << it->second.getBodySize() << std::endl;

			std::map<std::string, bool> tmp = it->second.getMethods();
			std::cout << "Methods: " << std::endl;
			for (std::map<std::string, bool>::iterator sit = tmp.begin(); sit != tmp.end(); ++sit) {
				std::cout << sit->first << ": " << sit->second << std::endl;
			}

			std::pair<int, std::string> redir = it->second.getRedir();
			std::cout << redir.first << ": " << redir.second << std::endl << std::endl;

			std::cout << "CGI_PATH: " << std::endl;
			std::multimap<std::string, std::string> s = it->second.getCGI();
			for (std::multimap<std::string, std::string>::iterator cit = s.begin(); cit != s.end(); ++cit) {
				std::cout << cit->first << ":" << cit->second << std::endl;
			}
			std::cout << std::endl << std::endl;
		}
	}
}