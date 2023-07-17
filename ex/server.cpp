#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <cstring>

int main() {
  // Create a socket (IPv4, TCP)
  int listenfd, connfd, n;
  struct  sockaddr_in servaddr;
  char  buff[200];
  char  bytes[200];
  
  listenfd = socket(AF_INET, SOCK_STREAM, 0); //  Internet Protocol v4 == AF_INET is an address family,SOCK_STREAM for TCP 
  if (listenfd < 0) {
    std::cout << "Failed to create socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Listen to port 9999 on any address;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // takes a 32-bit number in host byte order and returns a 32-bit number in the network byte order used in TCP/IP networks
  servaddr.sin_port = htons(9998); // htons is necessary to convert a number to
                                   // network byte order
  if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)  {
    std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Start listening. Hold at most 10 connections in the queue
  if (listen(listenfd, 10) < 0) {
    std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
    exit(EXIT_FAILURE);
  }

  // Grab a connection from the queue
  while(1)
  {

    std::cout << "Waiting for a connection on a port" << 9999 << std::endl;

    connfd = accept(listenfd, NULL, NULL);

    while( (n = read(connfd, bytes, 199)) > 0)
    {
      std::cout << " \n\n" << bytes;

      if (bytes[n - 1] == '\n')
        break;
      memset(bytes, 0, 200);
    }
    if (n < 0)
    {
      std::cout << "Failed to read. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }

    snprintf((char*)buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHello");

    write(connfd, (char *)buff, strlen(buff));
    close(connfd);

  }
  return (0);

}