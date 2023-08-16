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

void Server::sendHTMLResponse(int fd, std::string filepath) 
{
    char buff[200];
    std::string content_type;
    // Read the content of the HTML file
      std::cout << " \n In file returning \n";

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string suffix = filepath.substr(filepath.rfind(".") + 1);
    // Construct the HTTP response header
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
    snprintf(buff, sizeof(buff), "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", "text/html");
    // snprintf(buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\n");

    // Send the header to the client using send
    send(fd, buff, strlen(buff), 0);

    // Send the content of the HTML file to the client using send
    while (file.good()) {
        file.read(buff, sizeof(buff));
        int bytesRead = file.gcount();
        if (bytesRead > 0) {
            send(fd, buff, bytesRead, 0);
        }
    }

    // Close the file
    file.close();
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

        // const char* path_to_script = "/Users/cgreenpo/our_webserv/Config/cgi-bin/script";
        char* _args[] = {const_cast<char*>(script_path), nullptr};

        // char* _env[] = {
        //     const_cast<char*>("GATEWAY_INTERFACE=CGI/1.1"),
        //     const_cast<char*>("SERVER_SOFTWARE=Our_Web_Server/1.0"),
        //     const_cast<char*>("SERVER_NAME=example.com"),
        //     const_cast<char*>("SERVER_PORT=9999"),
        //     const_cast<char*>("REMOTE_ADDR=127.0.0.1"),
        //     const_cast<char*>("CONTENT_LENGTH=9"),
        //     const_cast<char*>("HTTP_USER_AGENT=Our_Web_Client/1.0"),
        //     const_cast<char*>("HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"),
        //     const_cast<char*>("HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.5"),
        //     const_cast<char*>("HTTP_ACCEPT_ENCODING=gzip, deflate"),
        //     const_cast<char*>("HTTP_CONNECTION=keep-alive"),
        //     const_cast<char*>("REQUEST_METHOD=POST"),

        //     const_cast<char*>("CLIENT_REQUEST=name=john"),
        //     const_cast<char*>("PATH_INFO=/Users/kris/our_webserve/ex/index.html"),
        //     NULL
        // };

        dup2(pipe_d[READ_END], STDIN_FILENO);
        close(pipe_d[READ_END]);

        execve(_args[0], _args, client.getReq().getENV());
        // Handle execve error...
    }

    close(pipe_d[READ_END]);
    close(outfile);

    // int status;
    // pid_t terminatedPid = waitpid(cgi_pid, &status, 0);
    // if (terminatedPid == -1) {
    //     std::cerr << "cgi: error with process handling\n";
    // }

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

    // Construct the HTTP response header
    snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", "text/html");

    // Send the header to the client using send
    send(fd, header, strlen(header), 0);
    send(fd, output.c_str(), output.size(), 0);

    outFileStream.close();
    remove(out_filename);

    remove(out_filename);
    close(fd); // Close the client socket
}

// void Server::launchCgi(class Client client)
// {
//     int infile = 0;
//     std::string body_path;
    

//     // Create a dynamic filename for the output of the Python script.
//     std::string tmp = client.getReq().getResource() + "output" + client.getClienIP();
//     const char* out_filename = tmp.c_str(); //dynamic filename

//     // Open the output file for writing (create if it doesn't exist with permissions 600).
//     int outfile = open(out_filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
//     if (outfile == -1) 
//     {
//         std::cerr << "cgi: Error opening the outfile.\n";
//         // throw (CgiException());
//     }

//     // Create a pipe for interprocess communication (IPC).
//     int pipe_d[2];
//     if (pipe(pipe_d) == -1)
//     {
//         std::cout << "Pipe Error\n";
//         // throw (CgiException());
//     }

//     // If there is data in the client request, set the size of the pipe buffer accordingly.
//     if (client.getReq().getBody().size() > 0)
//         fcntl(pipe_d[WRITE_END], 0, client.getReq().getBody().size());

//     // Write the client request data to the pipe.
//     write(pipe_d[WRITE_END], client.getReq().getBody().c_str(), client.getReq().getBody().size());
//     close(pipe_d[WRITE_END]);

//     // Fork a new process to run the Python script.
//     int cgi_pid = fork();
//     if (cgi_pid < 0)
//     {
//         close(pipe_d[READ_END]);
//         close(outfile);
//         std::cerr << "Error with fork\n";
//     }

//     // Code executed by the child process.
//     if (cgi_pid == 0)
//     {
//         // Redirect standard output to the output file.
//         dup2(outfile, STDOUT_FILENO);
//         close(outfile);

//         // If there is no query string, redirect standard input to the input file.
//         if (!client.getReq().getQueryString().c_str())
//         {
//             dup2(infile, STDIN_FILENO);
//             close(infile);
//         }

//         // Create arguments for the execve call to run the Python script.
//         // char* script_path = (char*)(client.path_on_server.c_str());

//         const char* path_to_python = "/usr/bin/python3";
//         char* _args[3];
//         _args[0] = (char*)path_to_python;
//         // _args[1] = script_path;
// 		// fixed by zyko
// 		std::string tmp = "/Users/cgreenpo/our_webserv/Config/cgi-bin/script";
//         _args[1] = new char[tmp.length() + 1];
// 		std::strcpy(_args[1], tmp.c_str());
//         _args[2] = NULL;




//         // Redirect standard input and output for the child process.
//         dup2(pipe_d[READ_END], STDIN_FILENO);
//         close(pipe_d[READ_END]);

//         dup2(outfile, STDOUT_FILENO);
//         close(outfile);



//         // // Set up a signal handler for the timeout.
//         // struct sigaction sa;
//         // memset(&sa, 0, sizeof(sa));
//         // sa.sa_handler = handleTimeout;
//         // sigaction(SIGALRM, &sa, NULL);

//         // // Set the timeout alarm.
//         // alarm(timeoutDuration);



//         // Execute the Python script with the given arguments and environment variables.
//         execve(_args[0], const_cast<char* const*>(_args), client.get_env());
//         // throw (CgiException()); // Throw an exception if execve fails.
//     }

//     // Code executed by the parent process.
//     int status;
//     close(pipe_d[READ_END]);
//     close(outfile);

//     // Wait for the child process to terminate and get its status.
//     pid_t terminatedPid = waitpid(cgi_pid, &status, 0);
//     if (terminatedPid == -1)
//     {
//         std::cerr << "cgi: error with process handling\n";
//         // throw (CgiException());
//     }

//     //---- TIMEOUT
//     // // Check if a timeout occurred and handle the child process accordingly.
//     // if (timeoutOccurred)
//     //     std::cerr << "Timeout occurred. Child process was terminated." << std::endl;
//     // else
//     // {
//     //     // Handle normal exit
//     //     if (WIFEXITED(status)) 
//     //     {
//     //         // Do further processing based on the status of the child process.
//     //         client.path_on_server = out_filename;
//     //         if (client.obtainFileLength())
//     //             client.response.generateCgiResponse();
//     //         else
//     //             throw (CgiException());
//     //     }
//     //     else if (WIFSIGNALED(status))
//     //     {
//     //         std::cerr << "Child process terminated due to signal: " << WTERMSIG(status) << std::endl;
//     //         // const char* cgi_error_path = "../HTML/cgi-bin/cgi_error.html";
//     //         throw (CgiException());
//     //     }
//     // }
// }


void Server::setUp()
{
    // std::cout << _conf.servers[0].host << std::endl;
    std::vector<int> port_numbers;
    // port_numbers.push_back(9999);
    // port_numbers.push_back(9998);
    // port_numbers.push_back(9997);
    port_numbers.push_back((int)_conf.servers[0].port[0]);
	// std::map<int, int> fd_to_port;
	// std::map<int, > fd_to_port;




    std::vector<int> listenfds;
    struct sockaddr_in servaddr;
    char buff[200];
    char bytes[4048];

    std::vector<pollfd> pollfds;

    // Create listen sockets and bind them to different port numbers
    for (size_t i = 0; i < port_numbers.size(); ++i) 
    {
        int listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd < 0) {
            std::cout << "Failed to create socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port_numbers[i]);
		int optval = 1;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            std::cout << "Failed to bind to port " << port_numbers[i] << ". errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        if (listen(listenfd, 10) < 0) {
            std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        // Add the listenfd to the pollfds array
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
                    Client cl(pollfds[i].fd, client_ip, _conf.servers[0]);
                    try
                    {
                       cl.readRequest();
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                    cl.print();
                    // if (cl.getReq().getCGIB())
                    if (1)
                        launchCgi(cl, fd);
                    else
                    {
                        sendHTMLResponse(fd, cl.getReq().getResource());

                    }

                    //----------------------
                    
                    
                    // if (bytes[n - 1] == '\n') {
                    //     close(fd);
                    //     sockets_to_remove.push_back(fd);
                    // } else {
                        // Send a response to the client
                        snprintf(buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHello");
                        write(pollfds[i].fd, buff, strlen(buff));
                    // }
                // }
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