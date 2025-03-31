# Webserv

## Overview
**Webserv** is a lightweight HTTP server built in **C++**, designed to mimic the functionality of **Nginx**. It supports **handling HTTP requests, serving static files, CGI execution, and multiple client connections**. The server is easily deployable using a **Makefile**.

## Features
### ✅ Completed Features:
- HTTP request handling (GET, POST, DELETE)
- Serving static files
- CGI support for dynamic content
- Multiple client connections
- Custom error pages
- Configurable through a configuration file
- Logging and request parsing
- Makefile for easy compilation and deployment

## Tech Stack
- **Language:** C++
- **Networking:** Sockets
- **Concurrency:** Select/Poll-based handling
- **Build System:** Makefile

## Setup Instructions
1. Clone the repository:
   ```sh
   git clone https://github.com/zyunusov-zy/webserv.git
   cd webserv
   ```
2. Compile the server:
   ```sh
   make
   ```
3. Run the server with a configuration file:
   ```sh
   ./webserv config/default.conf
   ```
4. Access the server in your browser at `http://localhost:8080` (or as specified in the configuration file).

## Configuration
Webserv uses a **configuration file** similar to Nginx. You can define:
- **Listening ports**
- **Root directories for serving files**
- **CGI scripts**
- **Error pages**
- **Request limits**

## Deployment
Since Webserv is compiled using **Makefile**, it can be deployed on any Unix-based system with minimal setup.

## Contributing
Contributions are welcome! Feel free to fork the repository and submit pull requests.

---
🚀 **Project Completed! Deploy your own HTTP server in C++!**

