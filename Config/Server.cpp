#include "Server.hpp"

Server::Server()
{
}

// void Server::conf(std::string filename, std::vector<t_serv>& servers)
// {
// 	// _conf.parse(filename, servers);
// 	// exit(1);
	
// }

Server::~Server()
{
}

#define WRITE_END 1
#define READ_END 0

void Server::sendHTMLResponse(class Client client, int fd, std::string filepath) 
{
    std::string content_type;
    // Read the content of the HTML file
    std::cout << " \n In html \n";
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
    }

    std::string extention = filepath.substr(filepath.rfind(".") + 1);
    if (extention == "html")
        content_type = "text/html";
    else if (extention == "css")
        content_type = "text/css";
    else if (extention == "txt")
        content_type = "text/plain";
    else if (extention == "ico")
        content_type = "image/x-icon";
    else if (extention == "jpg" || extention == "jpeg")
        content_type = "image/jpeg";
    else if (extention == "png")
        content_type = "image/png";
    else if (extention == "gif")
        content_type = "image/gif";
    else if (extention == "pdf")
        content_type = "application/pdf";
    else if (extention == "mp3")
        content_type = "audio/mpeg";
    else if (extention == "mp4")
        content_type = "audio/mpeg";
    else if (extention == "avi")
        content_type = "video/x-msvideo";
    else
        content_type = "application/octet-stream";

    client.getResp().setFilename(filepath);
    client.getResp().setFd(fd);
    client.getResp().sendResponse(content_type);

    file.close();
    std::cerr << "\n after file closing\n" << std::endl;

}


void Server::launchCgi(Client client, int fd) 
{
    int infile = 0; // Redirect input from /dev/null

    std::cerr << "\n\n ***** in CGI   \n";

    std::string string_filename = "output_file" + client.getClienIP();
    const char* out_filename = string_filename.c_str();

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

		if(client.getQuer() == false)
		{
			dup2(infile, STDIN_FILENO);
			close(infile);
		}
        char* script_path = (char*)(client.getReq().getUriCGI().c_str());
        const char* path_to_py = "/usr/local/bin/python3";
        char* _args[] = {const_cast<char*>(path_to_py), const_cast<char*>(script_path), nullptr};

        dup2(pipe_d[READ_END], STDIN_FILENO);
        close(pipe_d[READ_END]);
        if ((execve(_args[0], _args, client.getReq().getENV())) == -1)
            std::cerr << "\ncgi: error with execution\n";

        // Handle execve error...
    }

    close(pipe_d[READ_END]);
    // close(outfile);

    int status;
    pid_t terminatedPid = waitpid(cgi_pid, &status, 0);
    if (terminatedPid == -1) {
        std::cerr << "cgi: error with process handling\n";
    }

    client.getResp().setFilename(string_filename);
    client.getResp().setFd(fd);
    client.getResp().sendResponse("text/html");

    remove(out_filename);
    close(fd); // Close the client socket

}

void Server::setUp(std::vector<t_serv>& s)
{
    servers = s;
    std::vector<int> port_numbers;
	std::map<int, int> fd_to_port;
	std::map<int, t_serv*> port_to_serv;

    int i;
    int j;

    i = 0;
    while(i < servers.size())
    {
        j = 0;
        while (j < servers[i].port.size())
        {
            port_numbers.push_back((int)servers[i].port[j]);
            port_to_serv.insert(std::make_pair((int)servers[i].port[j], &(servers[i])));
            // std::cerr << "\n server struct pointer = " << port_to_serv[(int)_conf.servers[i].port[j]] << "\n";
            // std::cerr << "\n server struct value = " << port_to_serv[(int)_conf.servers[i].port[j]]->port[0] << "\n";

            j++;
        }
        i++;
    }

    std::vector<int> listenfds;
    struct sockaddr_in servaddr;
    char buff[200];
    char bytes[4048];

    std::vector<pollfd> pollfds;
    std::map<int, int> conn_to_listen;


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
            if (pollfds[i].revents & POLLIN) 
            {
                std::cout << "\n\nWaiting for a connection on port " << port_numbers[i] << std::endl;
                int connfd = accept(listenfds[i], NULL, NULL);
                if (connfd < 0) 
                {
                    std::cout << "Failed to accept connection. errno: " << errno << std::endl;
                    exit(EXIT_FAILURE);
                }
                connected_fds.push_back(connfd);
                std::cout << "Client connected on port: " << port_numbers[i] << std::endl;
                // Add the new connected socket to the pollfds array
                pollfd new_pollfd;
                new_pollfd.fd = connfd;
                new_pollfd.events = POLLIN; // Check for incoming data
                pollfds.push_back(new_pollfd);

                conn_to_listen.insert(std::make_pair(connfd, listenfds[i]));


            }
        }
        // Check connected client sockets for incoming data and handle them separately
        std::vector<int> sockets_to_remove;

        for (size_t i = listenfds.size(); i < pollfds.size(); ++i)
        {
            int fd = pollfds[i].fd;

            if (pollfds[i].revents & POLLIN) 
            {                    
                char client_ip[INET_ADDRSTRLEN];
                if(inet_ntop(AF_INET, &(servaddr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
                {
                    std::cerr << "Error converting IP address to string: " << strerror(errno) << std::endl;
                    exit(1);// need to change
                }

                std::cout << "HEL YEAH!" << std::endl;
                int listnfd_tmp = conn_to_listen[pollfds[i].fd];

                int port_tmp = fd_to_port[listnfd_tmp];
                t_serv  *serv_tmp = port_to_serv[port_tmp];
                if (serv_tmp)
                {
                    Client cl(pollfds[i].fd, client_ip, *serv_tmp);
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
                    {
                        launchCgi(cl, fd);
                        // cl.getResp().sendResponse();

                    }
                    else
                    {
                        sendHTMLResponse(cl, fd, cl.getReq().getResource());

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