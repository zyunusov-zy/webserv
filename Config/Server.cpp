#include "Server.hpp"

#define WRITE_END 1
#define READ_END 0

void Server::launchCgi(class Client client)
{
    int infile = 0;
    std::string body_path;
    

    // Create a dynamic filename for the output of the Python script.
    std::string tmp = client._req.getScriptName() + "output" + client.getClienIP();
    const char* out_filename = tmp.c_str(); //dynamic filename

    // Open the output file for writing (create if it doesn't exist with permissions 600).
    int outfile = open(out_filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (outfile == -1) 
    {
        std::cerr << "cgi: Error opening the outfile.\n";
        // throw (CgiException());
    }

    // Create a pipe for interprocess communication (IPC).
    int pipe_d[2];
    if (pipe(pipe_d) == -1)
    {
        std::cout << "Pipe Error\n";
        // throw (CgiException());
    }

    // If there is data in the client request, set the size of the pipe buffer accordingly.
    if (client._req.getBody().size() > 0)
        fcntl(pipe_d[WRITE_END], 0, client._req.getBody().size());

    // Write the client request data to the pipe.
    write(pipe_d[WRITE_END], client._req.getBody().c_str(), client._req.getBody().size());
    close(pipe_d[WRITE_END]);

    // Fork a new process to run the Python script.
    int cgi_pid = fork();
    if (cgi_pid < 0)
    {
        close(pipe_d[READ_END]);
        close(outfile);
        std::cerr << "Error with fork\n";
    }

    // Code executed by the child process.
    if (cgi_pid == 0)
    {
        // Redirect standard output to the output file.
        dup2(outfile, STDOUT_FILENO);
        close(outfile);

        // If there is no query string, redirect standard input to the input file.
        if (!client._req.getQueryString().c_str())
        {
            dup2(infile, STDIN_FILENO);
            close(infile);
        }

        // Create arguments for the execve call to run the Python script.
        // char* script_path = (char*)(client.path_on_server.c_str());

        const char* path_to_python = "/usr/bin/python3";
        char* _args[3];
        _args[0] = (char*)path_to_python;
        // _args[1] = script_path;
        _args[1] = "/Users/cgreenpo/our_webserv/Config/cgi-bin/script";      
        _args[2] = NULL;



        // // Set up environment variables required for the execution of the Python script.
        // std::string env_variable;
        // int i = 0;

        // // If there is a request body, set environment variables accordingly.
        // if (!client.query_string)
        // {
        //     env_variable = "CONTENT_LENGTH=" + client.request_header["Content-Length:"];
        //     enviromentals.push_back(env_variable);

        //     env_variable = "CONTENT_TYPE=" + client.request_header["Content-Type:"];
        //     enviromentals.push_back(env_variable);

        //     env_variable = "PATH_INFO=" + client.path_info;
        //     enviromentals.push_back(env_variable);
        // }
        // else
        // {
        //     // If there is a query string, set the QUERY_STRING environment variable.
        //     env_variable = "QUERY_STRING=" + client.request_header["query_string:"];
        //     enviromentals.push_back(env_variable);
        // }

        // // Set various CGI environment variables.
        // env_variable = "GATEWAY_INTERFACE=CGI/1.1";
        // enviromentals.push_back(env_variable);

        // env_variable = "REQUEST_METHOD=" + client.method;
        // enviromentals.push_back(env_variable);

        // env_variable = "REMOTE_ADDR=" + client.client_ip;
        // enviromentals.push_back(env_variable);

        // env_variable = "SCRIPT_NAME=" + client.request_header["location:"];
        // enviromentals.push_back(env_variable);

        // env_variable = "SERVER_NAME=" + client.server_name;
        // enviromentals.push_back(env_variable);

        // env_variable = "SERVER_PORT=" + client.config.port;
        // enviromentals.push_back(env_variable);

        // env_variable = "SERVER_PROTOCOL=HTTP/1.1";
        // enviromentals.push_back(env_variable);

        // env_variable = "SERVER_SOFTWARE=Weebserver";
        // enviromentals.push_back(env_variable);

        // char* _env[enviromentals.size() + 1];

        // // Convert environment variables to an array of char pointers.
        // for (std::vector<std::string>::iterator it = enviromentals.begin(); it != enviromentals.end(); it++)
        // {
        //     _env[i] = (char*)(*it).c_str();
        //     i++;
        // }
        // _env[i] = NULL;



        // Redirect standard input and output for the child process.
        dup2(pipe_d[READ_END], STDIN_FILENO);
        close(pipe_d[READ_END]);

        dup2(outfile, STDOUT_FILENO);
        close(outfile);



        // // Set up a signal handler for the timeout.
        // struct sigaction sa;
        // memset(&sa, 0, sizeof(sa));
        // sa.sa_handler = handleTimeout;
        // sigaction(SIGALRM, &sa, NULL);

        // // Set the timeout alarm.
        // alarm(timeoutDuration);



        // Execute the Python script with the given arguments and environment variables.
        execve(_args[0], const_cast<char* const*>(_args), client.get_env());
        // throw (CgiException()); // Throw an exception if execve fails.
    }

    // Code executed by the parent process.
    int status;
    close(pipe_d[READ_END]);
    close(outfile);

    // Wait for the child process to terminate and get its status.
    pid_t terminatedPid = waitpid(cgi_pid, &status, 0);
    if (terminatedPid == -1)
    {
        std::cerr << "cgi: error with process handling\n";
        // throw (CgiException());
    }

    //---- TIMEOUT
    // // Check if a timeout occurred and handle the child process accordingly.
    // if (timeoutOccurred)
    //     std::cerr << "Timeout occurred. Child process was terminated." << std::endl;
    // else
    // {
    //     // Handle normal exit
    //     if (WIFEXITED(status)) 
    //     {
    //         // Do further processing based on the status of the child process.
    //         client.path_on_server = out_filename;
    //         if (client.obtainFileLength())
    //             client.response.generateCgiResponse();
    //         else
    //             throw (CgiException());
    //     }
    //     else if (WIFSIGNALED(status))
    //     {
    //         std::cerr << "Child process terminated due to signal: " << WTERMSIG(status) << std::endl;
    //         // const char* cgi_error_path = "../HTML/cgi-bin/cgi_error.html";
    //         throw (CgiException());
    //     }
    // }
}


void Server::setUp()
{
    std::vector<int> port_numbers;
    // port_numbers.push_back(9999);
    // port_numbers.push_back(9998);
    // port_numbers.push_back(9997);
    port_numbers.push_back(_conf.servers[0].port);

    std::vector<int> listenfds;
    struct sockaddr_in servaddr;
    char buff[200];
    char bytes[200];

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
            if (pollfds[i].revents & POLLIN) {
                int fd = pollfds[i].fd;

                // Handle incoming data from the client
                int n = read(fd, bytes, sizeof(bytes) - 1);
                if (n < 0) {
                    std::cout << "Failed to read. errno: " << errno << std::endl;
                    exit(EXIT_FAILURE);
                }
                else if (n == 0) {
                    // Connection closed by the client
                    close(fd);
                    sockets_to_remove.push_back(fd);
                }
                else 
                {
                    bytes[n] = '\0';
                    std::cout << "\n\n" << bytes;
                    std::cout.flush();

                    
                    char client_ip[INET_ADDRSTRLEN];
                    if(inet_ntop(AF_INET, &(servaddr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
                    {
                        std::cerr << "Error converting IP address to string: " << strerror(errno) << std::endl;
                        exit(1);// need to change
                    }



                    Client cl(pollfds[i].fd, _conf, client_ip);
                    cl.print();
                    


                    //----------------------

                    // std::string method, url;
                    // parse_http_request(bytes, method, url);

                    // Execute the CGI script if the URL matches the CGI script path
                    // if (url == "/Users/cgreenpo/our_webserv/ex/cgi-bin/script") {

                    //     // Create pipes for communication with the CGI script
                    //     int pipefd[2];
                    //     if (pipe(pipefd) == -1) {
                    //         perror("pipe");
                    //         exit(EXIT_FAILURE);
                    //     }

                    //     // Fork again to execute the CGI script in a separate process
                    //     int cgi_pid = fork();
                    //     if (cgi_pid == 0) {
                    //         // Child process: close the read end of the pipe
                    //         close(pipefd[0]);

                    //         // Redirect stdout to the write end of the pipe
                    //         dup2(pipefd[1], STDOUT_FILENO);

                    //         // Close the write end of the pipe
                    //         close(pipefd[1]);

                    //         // Set up CGI environment variables
                    //         // ...

                    //         // Build the arguments array for execve
                    //         char* argv[] = { const_cast<char*>("/Users/cgreenpo/our_webserv/ex/cgi-bin/script"), NULL }; // Replace "/path/to/cgi_script" with the actual path to your CGI script

                    //         // Execute the CGI script
                    //         execve("/Users/cgreenpo/our_webserv/ex/cgi-bin/script", argv, initenv());

                    //         // If execve fails, print an error message and exit the child process
                    //         perror("execve");
                    //         exit(EXIT_FAILURE);
                    //     } else if (cgi_pid < 0) {
                    //         // Fork failed
                    //         std::cout << "Failed to fork. errno: " << errno << std::endl;
                    //         exit(EXIT_FAILURE);
                    //     } else {
                    //         // Parent process: close the write end of the pipe
                    //         close(pipefd[1]);

                    //         // Read the output from the CGI script
                    //         std::string cgi_output;
                    //         char buf[4096];
                    //         ssize_t bytes_read;
                    //         while ((bytes_read = read(pipefd[0], buf, sizeof(buf))) > 0) {
                    //             buf[bytes_read] = '\0';
                    //             cgi_output += buf;
                    //         }

                    //         // Close the read end of the pipe
                    //         close(pipefd[0]);

                    //         // Build the HTTP response with the CGI script's output
                    //         std::string response = "HTTP/1.0 200 OK\r\nContent-Length: " + std::to_string(cgi_output.length()) + "\r\n\r\n" + cgi_output;

                    //         // Send the response to the client
                    //         write(fd, response.c_str(), response.length());

                    //         // Close the client socket
                    //         close(fd);
                    //     }
                    //     continue; // Skip sending a response as the CGI script handles it
                    // }

                    // If the client sent '\n', close the connection
                    
                    
                    
                    if (bytes[n - 1] == '\n') {
                        close(fd);
                        sockets_to_remove.push_back(fd);
                    } else {
                        // Send a response to the client
                        snprintf(buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHello");
                        write(fd, buff, strlen(buff));
                    }
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


// char** initenv(void);
// // Function to execute a CGI script
// void execute_cgi(int fd, const std::string& method, const std::string& url) {
//     int pid = fork();
//     if (pid == 0) {
//         // Child process: execute the CGI script
//         dup2(fd, STDOUT_FILENO); // Redirect stdout to the client socket

//         // Set up CGI environment variables
//         // ...

//         // Build the arguments array for execve
//         char* argv[] = { const_cast<char*>("/Users/cgreenpo/our_webserv/ex/cgi-bin/script"), NULL }; // Replace "/path/to/cgi_script" with the actual path to your CGI script

//         // Execute the CGI script
//         execve("/Users/cgreenpo/our_webserv/ex/cgi-bin/script", argv, initenv());

//         // If execve fails, print an error message and exit the child process
//         perror("execve");
//         exit(EXIT_FAILURE);
//     } else if (pid < 0) {
//         // Fork failed
//         std::cout << "Failed to fork. errno: " << errno << std::endl;
//         exit(EXIT_FAILURE);
//     } else {
//         // Parent process
//         // Close the client socket in the parent process
//         close(fd);
//         int status;
//         waitpid(pid, &status, 0);
//     }
// }

// // Rest of the main function remains the same as before


// char** initenv(void) {
//     // Create an array of environment variables required by CGI script
//     // This array must be terminated with a NULL entry
//     char** _env = new char*[12];

//     _env[0] = const_cast<char*>("GATEWAY_INTERFACE=CGI/1.1");
//     _env[1] = const_cast<char*>("SERVER_SOFTWARE=Our_Web_Server/1.0");
//     _env[2] = const_cast<char*>("SERVER_NAME=example.com");
//     _env[3] = const_cast<char*>("SERVER_PORT=9999");
//     _env[4] = const_cast<char*>("REMOTE_ADDR=127.0.0.1");
//     _env[4] = const_cast<char*>("REQUEST_URI=/Users/cgreenpo/our_webserv/ex/index.html");
    
//     _env[5] = const_cast<char*>("CONTENT_LENGTH=1024");
//     _env[6] = const_cast<char*>("HTTP_USER_AGENT=Our_Web_Client/1.0");
//     _env[7] = const_cast<char*>("HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
//     _env[8] = const_cast<char*>("HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.5");
//     _env[9] = const_cast<char*>("HTTP_ACCEPT_ENCODING=gzip, deflate");
//     _env[10] = const_cast<char*>("HTTP_CONNECTION=keep-alive");
//     _env[11] = NULL;

//     return _env;
// }