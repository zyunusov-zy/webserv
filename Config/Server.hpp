#ifndef SERVER_HPP
# define SERVER_HPP

# define RED "\033[31m"
# define NORMAL "\033[0m"

#include <iostream>
// to set up a server
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include "Request.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "ErrorCodes.hpp"
#include <sys/wait.h>

#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>
#include <poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>



class Server
{
private:
	Config  _conf;

	std::string readFile(const std::string & filename);
public:
	Server(/* args */);
	~Server();

	void conf(std::string filename);
	void setUp();
	void launchCgi(Client cl, int fd);
    void sendHTMLResponse(int fd, std::string filepath);


};

Server::Server()
{
}

void Server::conf(std::string filename)
{
	_conf.parse(filename);
	// exit(1);
	
}

Server::~Server()
{
}

std::string Server::readFile(const std::string & filename)
{
	std::ifstream file(filename.c_str());
	std::stringstream buf;
	buf << file.rdbuf();
	return buf.str();
}

#define WRITE_END 1
#define READ_END 0

int calculateContentLength(std::ifstream& file) {
        file.seekg(0, std::ios::end);
        int length = static_cast<int>(file.tellg());
        file.seekg(0, std::ios::beg);
        return length;
    }

void Server::sendHTMLResponse(int fd, std::string filepath) 
{
    char buff[8192];
    char head[300];

    std::string content_type;
    // Read the content of the HTML file
      std::cout << " \n In file returning \n";


    filepath = "/Users/cgreenpo/our_webserv" + filepath;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
    }

    std::string suffix = filepath.substr(filepath.rfind(".") + 1);
    if (suffix == "html")
        content_type = "text/html";
    else if (suffix == "css")
        content_type = "text/css";
    else if (suffix == "txt")
        content_type = "text/plain";
    else if (suffix == "ico")
        content_type = "image/x-icon";
    else if (suffix == "jpg" || suffix == "jpeg")
        content_type = "image/jpeg";
    else if (suffix == "png")
        content_type = "image/png";
    else if (suffix == "gif")
        content_type = "image/gif";
    else if (suffix == "pdf")
        content_type = "application/pdf";
    else if (suffix == "mp3")
        content_type = "audio/mpeg";
    else if (suffix == "mp4")
        content_type = "audio/mpeg";
    else if (suffix == "avi")
        content_type = "video/x-msvideo";
    else
        content_type = "application/octet-stream";

    // snprintf(head, sizeof(head), "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", "text/html");

    snprintf(buff, sizeof(buff), "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: %s\r\n"
                                         "Content-Length: %d\r\n"
                                         "Connection: keep-alive\r\n"
                                         "\r\n", content_type.c_str(), calculateContentLength(file));


    send(fd, buff, strlen(buff), 0);

    // Send the content of the HTML file to the client using send
    // while (file.good()) {
    //     file.read(buff, sizeof(buff));
    //     int bytesRead = file.gcount();
    //     if (bytesRead > 0) {
    //         send(fd, buff, bytesRead, 0);
    //     }
    // }

    // int bytesRead = file.gcount();

    while (file.good())
    {
        file.read(buff, sizeof(buff));
        int bytesRead = file.gcount();
        if (bytesRead > 0) {
            int bytesSent = send(fd, buff, bytesRead, 0);
            if (bytesSent < 0) {
                std::cerr << "Send error" << std::endl;
                break;
            }
        }
        else 
        {
            // No more data to send
            break;
        }
        std::cerr << "\n in the loop\n" << std::endl;

    }

    file.close();
    std::cerr << "\n after file closing\n" << std::endl;

}


void Server::launchCgi(class Client client, int fd) 
{
    int infile = 0; // Redirect input from /dev/null

    std::cerr << "\n\n ***** in CGI   \n";

    const char* out_filename = "output_file";
    int outfile = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outfile == -1) {
        std::cerr << "cgi: Error opening the outfile.\n";
        // Handle error...
    }

    int pipe_d[2];
    if (pipe(pipe_d) == -1) {
        std::cerr << "Pipe Error\n";
    }

    write(pipe_d[WRITE_END], client.getReq().getBody().c_str(), client.getReq().getBody().size());
    close(pipe_d[WRITE_END]);

    int cgi_pid = fork();
    if (cgi_pid < 0) {
        close(pipe_d[READ_END]);
        close(outfile);
        std::cerr << "Error with fork\n";
        // Handle error...
    }

    if (cgi_pid == 0) {
        dup2(outfile, STDOUT_FILENO);
        close(outfile);

        dup2(infile, STDIN_FILENO);
        close(infile);

        char* script_path = (char*)(client.getReq().getUriCGI().c_str());
        // const char* path_to_script = "/Users/cgreenpo/our_webserv/Config/cgi-bin/script.py";
        const char* path_to_py = "/usr/local/bin/python3";

    


        // const char* path_to_script = "/Users/cgreenpo/our_webserv/Config/cgi-bin/script";
        char* _args[] = {const_cast<char*>(path_to_py), const_cast<char*>(script_path), nullptr};

        dup2(pipe_d[READ_END], STDIN_FILENO);
        close(pipe_d[READ_END]);

        if ((execve(_args[0], _args, client.getReq().getENV())) == -1)
            std::cerr << "\ncgi: error with execution\n";
        std::cerr << "\n\n ***** AFTER execution   \n";

        // Handle execve error...
    }

    close(pipe_d[READ_END]);
    close(outfile);

    int status;
    pid_t terminatedPid = waitpid(cgi_pid, &status, 0);
    if (terminatedPid == -1) {
        std::cerr << "cgi: error with process handling\n";
    }

    char header[200];

    std::ifstream outFileStream("output_file");
    if (!outFileStream) {
        std::cerr << "Failed to open output file for reading." << std::endl;
    }

    std::string output;
    std::string line;
    while (std::getline(outFileStream, line)) {
        output += line + "\n";
    }

    snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", "text/html");

    send(fd, header, strlen(header), 0);
    send(fd, output.c_str(), output.size(), 0);

    outFileStream.close();
    remove(out_filename);

    remove(out_filename);
    close(fd); // Close the client socket
}

void Server::setUp()
{
    // std::cout << _conf.servers[0].host << std::endl;
    std::vector<int> port_numbers;
    // port_numbers.push_back(9999);
    // port_numbers.push_back(9998);
    // port_numbers.push_back(9997);
    // port_numbers.push_back((int)_conf.servers[0].port[0]);

	std::map<int, int> fd_to_port;
	std::map<int, t_serv*> port_to_serv;

    int i;
    int j;

    i = 0;
    while(i < _conf.servers.size())
    {
        j = 0;
        while (j < _conf.servers[i].port.size())
        {
            port_numbers.push_back((int)_conf.servers[i].port[j]);
            std::cerr << "\n Pushed port  ." << _conf.servers[i].port[j]<< "\n";

            port_to_serv.insert(std::make_pair((int)_conf.servers[i].port[j], &(_conf.servers[i])));
            j++;
        }
        i++;
    }





    std::vector<int> listenfds;
    struct sockaddr_in servaddr;
    char buff[200];
    char bytes[4048];

    std::vector<pollfd> pollfds;

    // Create listen sockets and bind them to different port numbers
    for (size_t i = 0; i < port_numbers.size(); ++i) 
    {
        int listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd < 0) 
        {
            std::cout << "Failed to create socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port_numbers[i]);
		int optval = 1;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
        {
            std::cout << "Failed to bind to port " << port_numbers[i] << ". errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        if (listen(listenfd, 10) < 0) {
            std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        fd_to_port.insert(std::make_pair(listenfd, port_numbers[i]));

        pollfd listen_pollfd;
        listen_pollfd.fd = listenfd;
        listen_pollfd.events = POLLIN; // Check for incoming data
        pollfds.push_back(listen_pollfd);
        listenfds.push_back(listenfd);
    }

    std::vector<int> connected_fds;

    // Main server loop
    while (1) 
    {
        int activity = poll(&pollfds[0], pollfds.size(), -1); // Wait indefinitely until an event occurs
        if (activity < 0) {
            std::cout << "Error in poll. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        // Check for new connections on each listen socket

        for (size_t i = 0; i < listenfds.size(); ++i) 
        {
            if (pollfds[i].revents & POLLIN) {
                std::cout << "\n\nWaiting for a connection on port " << port_numbers[i] << std::endl;
                int connfd = accept(listenfds[i], NULL, NULL);
                if (connfd < 0) {
                    std::cout << "Failed to accept connection. errno: " << errno << std::endl;
                    exit(EXIT_FAILURE);
                }
                connected_fds.push_back(connfd);

                // Add the new connected socket to the pollfds array
                pollfd new_pollfd;
                new_pollfd.fd = connfd;
                new_pollfd.events = POLLIN; // Check for incoming data
                pollfds.push_back(new_pollfd);
            }
        }
        // Check connected client sockets for incoming data and handle them separately
        std::vector<int> sockets_to_remove;

        for (size_t i = listenfds.size(); i < pollfds.size(); ++i)
        {
            int fd = pollfds[i].fd;

            if (pollfds[i].revents & POLLIN) {
                // int fd = pollfds[i].fd;

                // // Handle incoming data from the client
                // int n = read(fd, bytes, sizeof(bytes) - 1);
                // if (n < 0) {
                //     std::cout << "Failed to read. errno: " << errno << std::endl;
                //     exit(EXIT_FAILURE);
                // }
                // else if (n == 0) {
                //     // Connection closed by the client
                //     close(fd);
                //     sockets_to_remove.push_back(fd);
                // }
                // else 
                // {
                    // bytes[n] = '\0';
                    // std::cout << "ARE YOU HERE?";
                    // std::cout << "\n\n" << bytes;
                    // std::cout.flush();

                    
                char client_ip[INET_ADDRSTRLEN];
                if(inet_ntop(AF_INET, &(servaddr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
                {
                    std::cerr << "Error converting IP address to string: " << strerror(errno) << std::endl;
                    exit(1);// need to change
                }


                // std::cout << "HERE11111" << std::endl;
                // std::cout << "File descriptor: " << pollfds[i].fd << std::endl;
                // std::cout << _conf.servers[0].host << std::endl;
                std::cout << "HEL YEAH!" << std::endl;
                int port_tmp = fd_to_port[pollfds[i].fd];

                Client cl(pollfds[i].fd, client_ip, *port_to_serv[port_tmp]);
                try
                {
                    cl.readRequest();
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
                cl.print();
                if (cl.getReq().getCGIB())
                    launchCgi(cl, fd);
                else
                {
                    sendHTMLResponse(fd, cl.getReq().getResource());

                }
                

            }
        }

        for (size_t i = 0; i < sockets_to_remove.size(); ++i) 
        {
            int fd = sockets_to_remove[i];

            // Remove the closed socket from the connected_fds vector
            for (size_t j = 0; j < connected_fds.size(); ++j) 
            {
                if (connected_fds[j] == fd) 
                {
                    connected_fds.erase(connected_fds.begin() + j);
                    break;
                }
            }

            // Remove the closed socket from the pollfds array
            for (std::vector<pollfd>::iterator it = pollfds.begin() + listenfds.size(); it != pollfds.end(); ++it) 
            {
                if (it->fd == fd) 
                {
                    pollfds.erase(it);
                    break;
                }
            }
        }

    }

}
#endif