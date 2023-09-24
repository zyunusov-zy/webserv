#include "Server.hpp"
#include <cstdio>

// #include <iostream>
#include <map>
#include <string>
Server::Server(){
}

Server::~Server(){
}

#define WRITE_END 1
#define READ_END 0


void handleFileUpload(const std::string& filename, const std::string& fileContent) {
    // Replace this with your logic to handle the file upload.
    // In a real web server, you would parse the request and save the file to a specified location.

    // Simulate saving the file content to a file on the server.
    std::ofstream outputFile(filename, std::ios::binary);
    if (outputFile.is_open()) {
        outputFile << fileContent;
        outputFile.close();
        std::cout << "File uploaded successfully.\n";
    } else {
        std::cerr << "Failed to upload the file.\n";
    }
}

#include <iostream>
#include <string>

std::string extractBoundary(const std::string& contentTypeHeader) {
    std::string boundary;
    
    size_t boundaryPos = contentTypeHeader.find("boundary=");
    if (boundaryPos != std::string::npos) {
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

    std::string filename = extractFilename(client.getReq().getBody());
    // Simulate receiving an HTTP request with a file upload.
    // std::string request = "POST /upload HTTP/1.1\r\n"
    //                       "Content-Length: 18\r\n"
    //                       "Content-Type: multipart/form-data; boundary=----boundary\r\n"
    //                       "\r\n"
    //                       "------boundary\r\n"
    //                       "Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
    //                       "\r\n"
    //                       "File content here\r\n"
    //                       "------boundary--";

    // std::string filename = client.getReq().getHeaders().(*begin)();

    // if (filename.length())
    // {
        std::map<std::string, std::string>::iterator it = client.getReq().getHeaders().begin();
        std::map<std::string, std::string> tmp;

        std::string boundary = extractBoundary(client.getReq().getHeaders()["Content-Type"]);


        tmp = client.getReq().getHeaders(); 
        std::string val = it->second;
        std::string requestBody = client.getReq().getBody();
        std::cerr << "filename" << "\n";
        std::cerr << filename << "\n";
        std::cerr << boundary << "\n";



    // Find the positions of the boundaries
    size_t start = requestBody.find("\r\n\r\n") + 4;
    size_t end = requestBody.rfind(boundary);

    if (start == std::string::npos || end == std::string::npos) {
        std::cerr << "Boundary not found in request body." << std::endl;
        return;
    }

    // Find the position of the end of the last boundary
    size_t lastBoundaryEnd = requestBody.rfind("\n", end);

    // Extract the content between the boundaries
    std::string content = requestBody.substr(start + boundary.length(), lastBoundaryEnd - (start + boundary.length()));

    // Print the extracted content
    std::cout << "Extracted Content:\n" << content << std::endl;
    // return ;

    // // std::string boundary = "------WebKitFormBoundarySYw3J90peFMr6zy3";

    // // Find the position of the first occurrence of the boundary
    // size_t start = fileContent.find(boundary);
    // if (start == std::string::npos) {
    //     std::cerr << "Boundary not found in request body." << std::endl;
    //     return;
    // }

    // // Find the position of the end of the headers
    // size_t headersEnd = fileContent.find("\n\n", start);
    // if (headersEnd == std::string::npos) {
    //     std::cerr << "Headers end not found in request body." << std::endl;
    //     return;
    // }

    // // Extract the content after the headers
    // std::string content = fileContent.substr(headersEnd + 2);

    // // Print the extracted content
    // std::cout << "Extracted Content:\n" << content << std::endl;


        // Handle the file upload.
        handleFileUpload(filename, content);
    // }

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



// void Response::generateDeleteResponse()
// {
// 	status_code = "204 No Content";
// 	content_type = "text/plain";
// 	additional_info.clear();
// 	content_length = 0;
// 	buildResponseHeader();
// }

void sendDeleteResponse(class Client client, int fd, std::string filepath)
{
	int i = std::remove(filepath.c_str());
	// if (i != 0)
	// {
	// 	fds_clients.at(client_fd).setError("409");
	// 	return;
	// }
	// fds_clients.at(client_fd).response.generateDeleteResponse();
    // client.getResp().sendResponse(content_type);

	// fds_clients.at(client_fd).request_processed = true;
}

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
        // const char* path_to_py = "/usr/bin/php";

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

    int i, j;

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
    char buff[200];
    char bytes[4048];

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
    while (1) {
        int activity = poll(&pollfds[0], pollfds.size(), -1); // Wait indefinitely until an event occurs
        if (activity < 0) {
            std::cout << "Error in poll. errno: " << errno << std::endl;
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < listenfds.size(); ++i) {
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
                pollfd new_pollfd; // Add the new connected socket to the pollfds array
                new_pollfd.fd = connfd;
                new_pollfd.events = POLLIN; // Check for incoming data
                pollfds.push_back(new_pollfd);
                conn_to_listen.insert(std::make_pair(connfd, listenfds[i]));
            }
        }
        std::vector<int> sockets_to_remove;// Check connected client sockets for incoming data and handle them separately
        for (size_t i = listenfds.size(); i < pollfds.size(); ++i){
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
                if (serv_tmp) {
                    Client cl(pollfds[i].fd, client_ip, *serv_tmp);
                    try {
                        cl.readRequest();
                        cl.print();
                        if (cl.checkError())
                        {
                            std::cout << "I Am HERE \n";
                            if (cl.getReq().getCGIB())
                                launchCgi(cl, fd);
                            else if (cl.getReq().getMethod() == "GET")
                                sendHTMLResponse(cl, fd, cl.getReq().getResource());
                            else if (cl.getReq().getMethod() == "POST")
                                sendPostResponse(cl, fd, cl.getReq().getResource());
                            else if (cl.getReq().getMethod() == "DELETE")
                                sendDeleteResponse(cl, fd, cl.getReq().getResource());

                        }
                    } catch (const std::exception& e) {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
        }
        for (size_t i = 0; i < sockets_to_remove.size(); ++i) {
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
