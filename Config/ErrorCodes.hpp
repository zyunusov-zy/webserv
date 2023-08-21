#ifndef ERROR_CODES_HPP
#define ERROR_CODES_HPP

#include <stdexcept>
#include <sys/stat.h>

#define RECV_BUFFER_SIZE 4048
#define STARTL 0
#define HEADERL 1
#define BODYL 2
#define END_STAT 3

#define BADREQUEST 400
#define OK 200
#define REQUEST_ENTITY_TOO_LARGE 413
# define UNABLE_TO_FIND 404



class ErrorException : public std::exception {

public :

	ErrorException(const char *msg) : errorMsg(msg), status(0) {}
	ErrorException(int st, const char *msg) : errorMsg(msg), status(st) {}
	int getStatus() const {return status;}

	virtual const char* what(void) const throw () {
		return (this->errorMsg);
	}

private:

	const char*	errorMsg;
	const int	status;

};

bool	isCharWhiteSpace(unsigned char c) {
	return std::isspace(c);
}

std::size_t	skipWhiteSpaces(std::string const &str, std::size_t start) {

	if (start >= str.length())
		return str.length();
	while (start < str.length() and isCharWhiteSpace(str[start]))
		start++;
	return start;
}

void trim(std::string& s, char c)
{
	s.erase(std::remove(s.begin(), s.end(), c), s.end());
}

void trimBegin(std::string & v)
{
	v.erase(v.begin(), std::find_if(v.begin(), v.end(),
							 std::bind1st(std::not_equal_to<char>(), ' ')));
	v.erase(v.begin(), std::find_if(v.begin(), v.end(),
							 std::bind1st(std::not_equal_to<char>(), '\t')));
}

std::uint8_t isDirOrFile(const char *path)
{
	std::cout << "CGI_PPPP: " << path << std::endl; 
	struct stat s;
	if (stat(path, &s) == -1)
		return 0;
	if (s.st_mode & S_IFDIR)
		return 2; // dir
	if (s.st_mode & S_IFREG)
		return 1; // file
	return 3;
}

#endif