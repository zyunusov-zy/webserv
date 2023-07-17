#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>

class Config
{
private:
	/* data */
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
}

int Config::parse(std::string fileName)
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

	while (token != nullptr) {
		tokens.push_back(token);
		token = std::strtok(nullptr, "\n\t");
	}

	// std::cout << "Hello" << std::endl;
	for (int i = 0; i < tokens.size(); i++) {
		size_t pos = tokens[i].find_first_not_of(' ');
		if (pos != std::string::npos){
			tokens[i].erase(0, pos);
		}
	}

	for (const auto& t : tokens) {
    std::cout << t << std::endl;
	}


	delete[] tmp;
	exit(0);
}

#endif