#ifndef ERROR_CODES_HPP
#define ERROR_CODES_HPP

#include <stdexcept>

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

#endif