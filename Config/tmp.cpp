void Server::launchCgi(class Client client, int fd) 
{
    int infile = 0; // Redirect input from /dev/null


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

    write(pipe_d[WRITE_END], "name=john", 9);
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

        const char* path_to_script = "/Users/cgreenpo/our_webserv/Config/cgi-bin/script";
        char* _args[] = {const_cast<char*>(path_to_script), nullptr};

        char* _env[] = {
            const_cast<char*>("GATEWAY_INTERFACE=CGI/1.1"),
            const_cast<char*>("SERVER_SOFTWARE=Our_Web_Server/1.0"),
            const_cast<char*>("SERVER_NAME=example.com"),
            const_cast<char*>("SERVER_PORT=9999"),
            const_cast<char*>("REMOTE_ADDR=127.0.0.1"),
            const_cast<char*>("CONTENT_LENGTH=9"),
            const_cast<char*>("HTTP_USER_AGENT=Our_Web_Client/1.0"),
            const_cast<char*>("HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"),
            const_cast<char*>("HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.5"),
            const_cast<char*>("HTTP_ACCEPT_ENCODING=gzip, deflate"),
            const_cast<char*>("HTTP_CONNECTION=keep-alive"),
            const_cast<char*>("REQUEST_METHOD=POST"),

            const_cast<char*>("CLIENT_REQUEST=name=john"),
            const_cast<char*>("PATH_INFO=/Users/kris/our_webserve/ex/index.html"),
            NULL
        };

        dup2(pipe_d[READ_END], STDIN_FILENO);
        close(pipe_d[READ_END]);

        execve(_args[0], _args, _env);
        std::cerr << "Execve error\n";
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

        std::ifstream outFileStream(out_filename);
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