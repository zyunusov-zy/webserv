#include "Server.hpp"
#include <cstdio>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <signal.h>

Server::Server()
{
    servers = std::vector<t_serv>();
}

Server::~Server(){
}

#define WRITE_END 1
#define READ_END 0

std::string extractFilename(const std::string& contentDispositionHeader) {
    std::string filename;
    size_t filenamePos = contentDispositionHeader.find("filename=");
    if (filenamePos != std::string::npos) {
        filenamePos += 10;
        size_t endPos = contentDispositionHeader.find("\"", filenamePos);
        if (endPos != std::string::npos) {
            filename = contentDispositionHeader.substr(filenamePos, endPos - filenamePos);
        }
    }
    return filename;
}

int handleFileUpload(const std::string& filename, const std::string& fileContent, const size_t file_size)
{
    std::ofstream outputFile(filename.c_str(), std::ios::binary);
    if (outputFile.is_open()) {
        outputFile.write(fileContent.c_str(), file_size);
        outputFile.close();
        return (true);
    } else {
        std::cerr << "Failed to upload the file.\n";
        return (false);
    }
}

void Server::sendPostResponse(class Client *client)
{
    HeaderMap hm = client->getReq().getBodyHeaders();
    std::string filename = extractFilename(hm["Content-Disposition"]);
	filename = client->getServ().Mroot + filename;

    std::cerr << filename << std::endl;
    std::cerr << client->getReq().getBody() << std::endl;

    if (handleFileUpload(filename, client->getReq().getBody(), client->getReq().getBody().size()))
	{
		client->getResp()->status_code = "201 Created";
		client->getResp()->content_type = "text/plain";
		client->getResp()->body = "File was uploaded successfully";
	}
	else
	{
		client->getResp()->exec_err_code = 500;
		throw(returnError());
	}
}

bool Server::sendDeleteResponse(class Client *client, std::string filepath)
{
	int i = std::remove(filepath.c_str());
	if (i != 0)
	{
		client->getResp()->exec_err_code = 409;
		throw(returnError());
	}
	client->getResp()->status_code = "204 No Content";
	client->getResp()->body = "Deleted successfully";
	client->getResp()->content_type = "text/html";
	return 0;
}

void Server::sendHTMLResponse(class Client *client, std::string filepath)
{
    std::string content_type;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw ErrorException("Failed to open a file");
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

    client->getResp()->content_type = content_type;
    client->getResp()->filename = filepath;
    file.close();
}

static volatile sig_atomic_t timeoutOccurred = 0;

void handleTimeout(int signum) {
    (void)signum;
    timeoutOccurred = 1;
}

void cleanEnv(Client *client)
{
    if (client->getReq().getENV() != nullptr) {
        for (size_t i = 0; client->getReq().getENV()[i] != nullptr; ++i) {
            if (client->getReq().getENV()[i])
                free(client->getReq().getENV()[i]);
        }
        delete[] client->getReq().getENV();
    }
}

bool Server::launchCgi(Client *client)
{
    const int timeoutDuration = 3;
    int infile = 0;
    client->getResp()->exec_err_code = 500;

	std::stringstream tmp;
    tmp << client->fd;

    std::string string_filename = "output_file" + client->getClienIP() + tmp.str();
    client->getResp()->filename = string_filename;
    const char* out_filename = string_filename.c_str();
    int outfile = open(out_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (outfile == -1) {
        std::cerr << "cgi: Error opening the outfile.\n";
        return 1;
    }

    int pipe_d[2];
    if (pipe(pipe_d) == -1) {
        std::cerr << "Pipe Error\n";
        return 1;
    }

    write(pipe_d[WRITE_END], client->getReq().getBody().c_str(), client->getReq().getBody().size());
    close(pipe_d[WRITE_END]);

    int cgi_pid = fork();
    if (cgi_pid < 0) {
        close(pipe_d[READ_END]);
        close(outfile);
        std::cerr << "Error with fork\n";
        return 1;
    }

    if (cgi_pid == 0) {
        timeoutOccurred = 0;
        dup2(outfile, STDOUT_FILENO);
        close(outfile);

        if (client->getQuer() == false) {
            dup2(infile, STDIN_FILENO);
            close(infile);
        }
        char* script_path = (char*)(client->getReq().getUriCGI().c_str());
        const char* path_to_py = "/usr/local/bin/python3";
        char* _args[] = {const_cast<char*>(path_to_py), const_cast<char*>(script_path), nullptr};

        dup2(pipe_d[READ_END], STDIN_FILENO);
        close(pipe_d[READ_END]);

        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handleTimeout;

        if (sigaction(SIGALRM, &sa, NULL) == -1) {
            perror("sigaction");
            return 1;
        }
        alarm(timeoutDuration);

        if ((execve(_args[0], _args, client->getReq().getENV())) == -1) {
            std::cerr << "\n cgi: error with execution\n";
            return 1;
        }
    }
    client->getResp()->exec_err_code = 504;

    close(pipe_d[READ_END]);
    int status;
    pid_t terminatedPid = waitpid(cgi_pid, &status, 0);
    if (terminatedPid == -1) {
        std::cerr << "cgi: error with process handling\n";
        return 1;
    }

    if (WIFEXITED(status))
    {
        std::cerr << "Child process exited with status: " << WEXITSTATUS(status) << std::endl;
    }
    else if (WIFSIGNALED(status))
    {
        std::cerr << "Child process terminated due to signal: " << WTERMSIG(status) << std::endl;
        return 1;
    }
    client->getResp()->content_type = "text/html";
    alarm(0);
    return 0;
}

void removeSocket(int fd, std::vector<int>& connected_fds,
                  std::vector<int>& sockets_to_remove,
                  std::map<int, Client*>& fd_to_clients,
                  std::vector<pollfd>& pollfds,
                  const std::vector<int>& listenfds) {

    for (size_t j = 0; j < connected_fds.size(); ++j) {
        if (connected_fds[j] == fd) {
            connected_fds.erase(connected_fds.begin() + j);
            break;
        }
    }

    std::vector<int>::iterator it = std::find(sockets_to_remove.begin(), sockets_to_remove.end(), fd);
    if (it != sockets_to_remove.end()) {
        sockets_to_remove.erase(it);
    }

    std::map<int, Client*>::iterator clp = fd_to_clients.find(fd);
    if (clp != fd_to_clients.end()) {
        delete clp->second->_resp;
        delete clp->second;
        fd_to_clients.erase(clp);
    }

    for (std::vector<pollfd>::iterator it = pollfds.begin() + listenfds.size(); it != pollfds.end(); ++it) {
        if (it->fd && it->fd == fd) {
            pollfds.erase(it);
            break;
        }
    }
}



void Server::setUp(std::vector<t_serv>& s)
{
    servers = s;
    std::vector<int> port_numbers;
    std::map<int, int> fd_to_port;
    std::map<int, t_serv*> port_to_serv;

    size_t i, j;

    i = 0;
    while(i < servers.size())
    {
        j = 0;
        while (j < servers[i].port.size())
        {
            port_numbers.push_back((int)servers[i].port[j]);
            port_to_serv.insert(std::make_pair((int)servers[i].port[j], &(servers[i])));
            j++;
        }
        i++;
    }

    std::vector<int> listenfds;
    struct sockaddr_in servaddr;

    std::vector<pollfd> pollfds;
    std::map<int, int> conn_to_listen;

    for (size_t i = 0; i < port_numbers.size(); ++i) {
        int listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd < 0) 
        {
            std::cerr << "Failed to create socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port_numbers[i]);
        int optval = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
        {
            std::cerr << "Failed to bind to port " << port_numbers[i] << ". errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }

        if (listen(listenfd, 10) < 0) {
            std::cerr << "Failed to listen on socket. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        fd_to_port.insert(std::make_pair(listenfd, port_numbers[i]));
        pollfd listen_pollfd;
        listen_pollfd.fd = listenfd;
        listen_pollfd.events = POLLIN;
        listen_pollfd.revents = 0;
        pollfds.push_back(listen_pollfd);
        listenfds.push_back(listenfd);
    }
    std::vector<int> connected_fds;
    while (1)
    {
        alarm(0);
        std::cerr << "in main LOOP" << '\n';

        int activity = poll(&pollfds[0], pollfds.size(), 0);
        if (activity < 0) {
            std::cerr << "Error in poll. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < listenfds.size(); ++i)
        {
            if (pollfds[i].revents & POLLIN) 
            {
                std::cout << "\n\nWaiting for a connection on port " << port_numbers[i] << std::endl;
                int connfd = accept(listenfds[i], NULL, NULL);
                if (connfd < 0) 
                {
                    std::cerr << "Failed to accept connection. errno: " << errno << std::endl;
                    exit(EXIT_FAILURE);
                }
                connected_fds.push_back(connfd);
                pollfd new_pollfd;
                new_pollfd.fd = connfd;
                new_pollfd.events = POLLIN;
                pollfds.push_back(new_pollfd);
                conn_to_listen.insert(std::make_pair(connfd, listenfds[i]));
            }
        }
        std::vector<int> sockets_to_remove;
        for (size_t i = listenfds.size(); i < pollfds.size(); ++i)
        {
            int fd = pollfds[i].fd;
            if (pollfds[i].revents & POLLIN & fd_to_clients.count(pollfds[i].fd) > 0)
            {
                try
                {
                    std::cerr << "\nKEEP READING" << '\n';
                    if (fd_to_clients.find(fd)->second->getToServe() == true)
                        pollfds[i].events = POLLOUT;
                    else
                    {
                        fd_to_clients.find(fd)->second->readRequest();
                    }

                    if (fd_to_clients.find(fd)->second->getToServe() == true)
                    {
                        std::cerr << "\nSTOP READING" << '\n';
                        pollfds[i].events = POLLOUT;
                    }
                    std::cerr << "\nEND KEEP READING" << '\n';
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                    fd_to_clients.find(fd)->second->checkError();
                    sockets_to_remove.push_back(pollfds[i].fd);
                }
            }
            else if (pollfds[i].revents & POLLIN & fd_to_clients.count(pollfds[i].fd) == 0) 
            {
                char client_ip[INET_ADDRSTRLEN];
                if(inet_ntop(AF_INET, &(servaddr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
                {
                    std::cerr << "Error converting IP address to string: " << strerror(errno) << std::endl;
                    // exit(1);
                }
                int listnfd_tmp = conn_to_listen[pollfds[i].fd];
                int port_tmp = fd_to_port[listnfd_tmp];
                t_serv  *serv_tmp = port_to_serv[port_tmp];
                if (serv_tmp)
                {
                    Client* myCl = new Client(pollfds[i].fd, client_ip, *serv_tmp);
                    Response* myResp = new Response();
                    myCl->_resp = myResp;
                    fd_to_clients.insert(std::make_pair(pollfds[i].fd, myCl));
                    if (myCl->getIsClosed() == true)
                    {
                        sockets_to_remove.push_back(pollfds[i].fd);
                    }
                    else
                    {
                        try
                        {
                            myCl->readRequest();
                            // myCl->print();
                            myCl->pollstruct = &(pollfds[i]);
                            if (myCl->checkError())
                            {
                                std::cout << "I Am HERE \n";
                                if (myCl->getReq().getCGIB())
                                {
                                    if (launchCgi(myCl))
                                    {
                                        myCl->getResp()->exec_err = true;
                                        remove(myCl->getResp()->filename.c_str());
                                        throw(returnError());
                                    }
                                }
                                else if (myCl->getReq().getMethod() == "GET")
                                    sendHTMLResponse(myCl, myCl->getReq().getResource());
                                else if (myCl->getReq().getMethod() == "POST")
                                {
                                    std::cout << "BEFORE POST \n";
                                    if (myCl->getToServe() == true)
                                    {
                                        sendPostResponse(myCl);
                                        myCl->getResp()->post_done = true;
                                    }
                                }
                                else if (myCl->getReq().getMethod() == "DELETE")
                                    sendDeleteResponse(myCl, myCl->getReq().getResource());
                                if (myCl->getToServe() == true)
                                    pollfds[i].events = POLLOUT;
                                myCl->getResp()->_target_fd = fd;
                            }
                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << e.what() << '\n';
                            myCl->checkError();
                            sockets_to_remove.push_back(pollfds[i].fd);
                        }
                    }
                }
            }
            else if (pollfds[i].revents & POLLOUT)
            {
                std::cerr << "+++ GENERAL POLLOUTTTT" << '\n';
                client_it  = this->fd_to_clients.find(fd);
                if (client_it->second->getReq().getMethod() == "POST" && client_it->second->getResp()->post_done == false)
                {
                    try
                    {
                        sendPostResponse(client_it->second);
                    }
                    catch(const std::exception& e)
                    {
                        fd_to_clients.find(fd)->second->checkError();
                        sockets_to_remove.push_back(pollfds[i].fd);
                    }
                }
                if (client_it != this->fd_to_clients.end() && !client_it->second->getResp()->response_complete)
                {
                    std::cerr << "POLLOUTTTT send" << '\n';
                    if (client_it->second->getResp()->sendResponse(client_it->second->getResp()->content_type) == 1)
                    {
						std::cerr << "Error: Sending." << std::endl;

                        client_it->second->getResp()->exec_err_code = 500;
                        client_it->second->checkError();
                        sockets_to_remove.push_back(pollfds[i].fd);
                    }
                }
                if (client_it != this->fd_to_clients.end() && client_it->second->getResp()->response_complete)
                {
                    std::cerr << "POLLOUTTTT remove" << '\n';
                    if (client_it->second->getReq().getCGIB())
                    {
                        remove(client_it->second->getResp()->filename.c_str());
                        std::cerr << client_it->second->getResp()->filename.c_str() << '\n';
                    }
                    sockets_to_remove.push_back(pollfds[i].fd);
                }
            }
        }
        for (size_t i = 0; i < sockets_to_remove.size(); ++i)
        {
            std::cerr << "in Removing" << '\n';
            int fd = sockets_to_remove[i];
            if (sockets_to_remove.size() > 1)
                i--;
            removeSocket(fd, connected_fds, sockets_to_remove, fd_to_clients, pollfds, listenfds);
            close(fd);
        }
    }
}
