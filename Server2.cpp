#include "Server.hpp"

#include <cstdio>

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>

Server::Server(){
}

Server::~Server(){
}

#define WRITE_END 1
#define READ_END 0


void handleFileUpload(const std::string& filename, const std::string& fileContent, const size_t file_size, const size_t upload_header_size) 
{

    std::ofstream outputFile(filename.c_str(), std::ios::binary | std::ios::trunc);
    // std::ofstream outputFile(filename, std::ios::binary);
    if (outputFile.is_open()) {
        // outputFile << fileContent;
    // size_t file_size = ds_clients.at(client_fd).request.rfind(boundary) - upload_header_size;

    // Write the uploaded file data to the file
    outputFile.write(fileContent.substr(upload_header_size, file_size).c_str(), file_size);

        outputFile.close();
        std::cout << "File uploaded successfully.\n";
    } else {
        std::cerr << "Failed to upload the file.\n";
    }
}

// #include <iostream>
// #include <string>

std::string extractBoundary(const std::string& contentTypeHeader) {
    std::string boundary;
    
    size_t boundaryPos = contentTypeHeader.find("boundary=");
    if (boundaryPos != std::string::npos) 
    {
        boundaryPos += 9; // Move to the start of the boundary
        size_t semicolonPos = contentTypeHeader.find(";", boundaryPos);
        if (semicolonPos != std::string::npos) {
            // Extract the boundary value up to the semicolon
            boundary = contentTypeHeader.substr(boundaryPos, semicolonPos - boundaryPos);
        } else {
            // If no semicolon found, extract the boundary value until the end of the string
            boundary = contentTypeHeader.substr(boundaryPos);
        }
    }
    return boundary;
}




std::string extractFilename(const std::string& contentDispositionHeader) {
    std::string filename;
    
    size_t filenamePos = contentDispositionHeader.find("filename=");
    if (filenamePos != std::string::npos) {
        filenamePos += 10; // Move to the start of the filename
        size_t endPos = contentDispositionHeader.find("\"", filenamePos);
        if (endPos != std::string::npos) {
            // Extract the filename
            filename = contentDispositionHeader.substr(filenamePos, endPos - filenamePos);
        }
    }

    return filename;
}

void sendPostResponse(class Client client, int fd, std::string filepath)
 {
    std::cout << "IN POST RESP ----- \n\n";

    size_t upload_header_size;
    std::string upload_header;
    std::string filename = extractFilename(client.getReq().getBody());

    upload_header_size = client.getReq().getBody().find("\r\n\r\n") + 4;
    upload_header = client.getReq().getBody().substr(0, upload_header_size);

    // std::ofstream createFile(file_path.c_str(), std::ios::binary | std::ios::trunc);

    std::map<std::string, std::string> tmp;
    std::ifstream dmm(client.getReq().getBody());

    std::string boundary = extractBoundary(client.getReq().getHeaders()["Content-Type"]);


    tmp = client.getReq().getHeaders(); 
    std::string requestBody = client.getReq().getBody();
    std::cerr << "filename" << "\n";
    std::cerr << filename << "\n";
    std::cerr << boundary << "\n";



    
    // std::cout << "Extracted Content:\n" << tmp << std::endl;
    size_t file_size = requestBody.rfind("\r\n--" + boundary + "--") - upload_header_size;

    handleFileUpload(filename, requestBody, file_size, upload_header_size);


    client.getResp().status_code = "201 Created";
	client.getResp().content_type = "text/plain";
	client.getResp().body = "File was uploaded succesfully";
	client.getResp().content_len = client.getResp().body.size();
	client.getResp().additional_info = "Location: " + filepath;

    client.getResp().sendResponse("text/plain");

}


// void sendPostResponse(class Client client, int fd, std::string filepath)
// {
//     client.getQuer()

//     std::string upload_header;
// 	size_t upload_header_size;


// 	upload_header_size = fds_clients.at(client_fd).request.find("\r\n\r\n") + 4;
// 	upload_header = fds_clients.at(client_fd).request.substr(0, upload_header_size);

// 	std::string boundary_end = "\r\n--" + fds_clients.at(client_fd).request_header.at("boundary") + "--";


// 	std::string filename = upload_header.substr(upload_header.find("filename=") + 10);
// 	filename = filename.substr(0, filename.find("\""));
// 	if (!validateFilename(filename))
// 	{
// 		fds_clients.at(client_fd).setError("400");
// 		return;
// 	}
// 	std::string file_path = fds_clients.at(client_fd).path_on_server + filename;
// 	std::ofstream createFile(file_path.c_str(), std::ios::binary | std::ios::trunc);
// 	if (!createFile.is_open())
// 	{
// 		fds_clients.at(client_fd).setError("500");
// 		return;
//     }
// }



void sendDeleteResponse(class Client client, int fd, std::string filepath)
{
	int i = std::remove(filepath.c_str());
	// if (i != 0)
	// {
	// 	fds_clients.at(client_fd).setError("409");
	// 	return;
	// }
    client.getResp().status_code = "204 No Content";
	client.getResp().content_type = "text/plain";
	// additional_info.clear();
	client.getResp().content_len = 0;

    client.getResp().sendResponse("text/plain");
	client.getResp().response_complete = true;
}



void Server::sendHTMLResponse(class Client client, int fd, std::string filepath) 
{
    std::string content_type;

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


    client.getResp().content_type = content_type;
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
    }
    close(pipe_d[READ_END]);

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

        if (listen(listenfd, 10) < 0) 
        {
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
    std::unordered_map<int, Client> fd_to_client;
    t_serv  *serv_tmp = NULL;
    int connfd;

    while (1) 
    {
        int activity = poll(&pollfds[0], pollfds.size(), -1); // Wait indefinitely until an event occurs
        if (activity < 0) {
            std::cout << "Error in poll. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < listenfds.size(); ++i) 
        {
            if (pollfds[i].revents & POLLIN) 
            {
                std::cout << "\n\nWaiting for a connection on port " << port_numbers[i] << std::endl;
                connfd = accept(listenfds[i], NULL, NULL);
                if (connfd < 0) 
                {
                    std::cout << "Failed to accept connection. errno: " << errno << std::endl;
                    exit(EXIT_FAILURE);
                }
                connected_fds.push_back(connfd);
                std::cout << "Client connected on port: " << port_numbers[i] << std::endl;
                pollfd new_pollfd; // Add the new connected socket to the pollfds array
                new_pollfd.fd = connfd;
                new_pollfd.events = POLLIN; // Check for incoming data
                pollfds.push_back(new_pollfd);
                conn_to_listen.insert(std::make_pair(connfd, listenfds[i]));
                std::cout << "!!!!!!!: " << port_numbers[i] << std::endl;
				std::cout << connfd << std::endl;
				std::cout << listenfds[i] << std::endl;


                char client_ip[INET_ADDRSTRLEN];
                if(inet_ntop(AF_INET, &(servaddr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL)
                {
                    std::cerr << "Error converting IP address to string: " << strerror(errno) << std::endl;
                    // exit(1);// need to change
                }
                std::cout << "BEFORE CL INIT!" << std::endl;

                // int listnfd_tmp = conn_to_listen[pollfds[i].fd];
				// std::cout << "1!" << std::endl;
				// std::cout << listnfd_tmp << std::endl;


                // int port_tmp = fd_to_port[listnfd_tmp];
				int port_tmp = fd_to_port[listenfds[i]];

				std::cout << "2!" << std::endl;
				std::cout << port_tmp << std::endl;

                serv_tmp = port_to_serv[port_tmp];
				std::cout << "3!" << std::endl;
				std::cout << serv_tmp->name << std::endl;

				if (serv_tmp == NULL)
					exit(0);

                Client cl(connfd, client_ip, *serv_tmp);
				std::cout << "4!" << std::endl;
				// exit(0);

                // fd_to_client.insert({connfd, cl});
                // fd_to_client[connfd] = cl;
                // fd_to_client.insert(std::make_pair(connfd, cl));
                fd_to_client.insert(std::pair<int, Client>(connfd, cl));
				std::cout << "AFTER CL INIT!" << std::endl;


            }
        }
        std::vector<int> sockets_to_remove;// Check connected client sockets for incoming data and handle them separately
        for (size_t i = listenfds.size(); i < pollfds.size(); ++i)
        {
            int fd = pollfds[i].fd;

 			std::cout << "AFTER unordered map!" << std::endl;

            if (pollfds[i].revents & POLLIN && client_it != fd_to_client.end()) 
            {
                // Client cl = fd_to_client[fd];
                std::cout << "HEL YEAH!" << std::endl;

                if (serv_tmp && client_it != fd_to_client.end()) 
                {
                    try 
                    {
                        client_it->second.readRequest();
						std::cout << "Read REquest! \n";

                        client_it->second.print();
                        if (client_it->second.checkError())
                        {
                            std::cout << "I Am HERE \n";
                            if (client_it->second.getReq().getCGIB())
                                launchCgi(client_it->second, fd);
                            else if (client_it->second.getReq().getMethod() == "GET")
                                sendHTMLResponse(client_it->second, fd, client_it->second.getReq().getResource());
                            else if (client_it->second.getReq().getMethod() == "POST")
                                sendPostResponse(client_it->second, fd, client_it->second.getReq().getResource());
                            else if (client_it->second.getReq().getMethod() == "DELETE")
                                sendDeleteResponse(client_it->second, fd, client_it->second.getReq().getResource());
                        }
                    } 
                    catch (const std::exception& e) 
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
            else if (pollfds[i].revents & POLLOUT && !client_it->second.getResp().response_complete)
            {
                client_it->second.getResp().sendResponse(client_it->second.getResp().content_type);
            }
            // if (fd_to_client[connfd].response_complete == true)
            // {
            //     add to disconnect;
                    // int sockets_to_remove.push(connfd);
                // fd_to_client.erase(connfd);
            // }
        }

        for (size_t i = 0; i < sockets_to_remove.size(); ++i) 
        {
            int fd = sockets_to_remove[i];
            for (size_t j = 0; j < connected_fds.size(); ++j)// Remove the closed socket from the connected_fds vector
            {
                if (connected_fds[j] == fd) 
                {
                    connected_fds.erase(connected_fds.begin() + j);
                    break;
                }
            }
            for (std::vector<pollfd>::iterator it = pollfds.begin() + listenfds.size(); it != pollfds.end(); ++it) {// Remove the closed socket from the pollfds array
                if (it->fd == fd) 
                {
                    pollfds.erase(it);
                    break;
                }
            }
        }
    }
}
