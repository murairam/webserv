# Webserv

A high-performance HTTP/1.1 web server in C++98 featuring event-driven architecture with poll-based I/O multiplexing. Part of the 42 school curriculum.

## Installation

```bash
git clone git@github.com:murairam/webserv.git
cd webserv
make
```

## Usage

```bash
./webserv [config.cfg]
```

## Overview

Webserv is a non-blocking HTTP server that handles concurrent connections efficiently using a single-threaded event loop. The server implements HTTP/1.1 core features including persistent connections, chunked encoding, virtual hosts, and CGI execution.

## Architecture

**EventLoop**: Central dispatcher using `poll()` for non-blocking I/O multiplexing. Monitors all file descriptors in a single system call with automatic timeout management.

**Connection**: Manages individual client connections and the complete HTTP request-response cycle. Integrated with EventLoop for efficient event handling.

**HttpRequestParser**: Incremental HTTP parsing supporting regular and chunked transfer encoding, handling partial reads and malformed requests gracefully.

**ConfigLoader**: Parses NGINX-style configuration files with support for virtual hosts, location-based routing, custom error pages, and CGI configuration.

**CgiHandler**: Manages CGI script execution through forked processes using non-blocking pipes integrated into the EventLoop.

**Response**: HTTP response construction with proper status codes, headers, MIME type detection, and directory listing generation.

## Features

### HTTP/1.1 Support
- GET, POST, DELETE methods
- Persistent connections (keep-alive)
- Chunked transfer encoding
- Request body size limits
- Accurate status codes

### Configuration
- Multiple virtual hosts
- Hostname-based routing
- Custom error pages
- Method restrictions per route
- Directory listings
- HTTP redirects

### CGI & File Operations
- Extension-based CGI routing (.php, .py)
- Environment variable setup per CGI spec
- Non-blocking CGI with timeout handling
- File upload/download/delete
- Static file serving with MIME types

## Installation

```bash
git clone [repository-url]
cd webserv
make
```

## Usage

```bash
./webserv [config.cfg]
```

### Configuration Example

```nginx
server {
    listen 8080;
    server_name localhost;

    client_max_body_size 10M;
    error_page 404 /errors/404.html;

    location / {
        root ./data/www;
        index index.html;
        allowed_methods GET POST DELETE;
    }

    location /cgi {
        root ./data/www/cgi;
        cgi .php /usr/bin/php-cgi;
        allowed_methods GET POST;
    }
}
```

## Testing

```bash
# Basic test
curl http://localhost:8080/

# Load test (requires siege: brew install siege)
siege -c 50 -t 30s http://localhost:8080/

# Memory leak test
valgrind --leak-check=full ./webserv config.cfg
```

## Technical Details

- Single `poll()` call per event loop iteration
- All socket operations are non-blocking (O_NONBLOCK)
- No `errno` checking after socket operations
- Signal handling via self-pipe technique
- Automatic idle timeout prevention
- Zero memory leaks (valgrind verified)

## Project Structure

```
srcs/
├── main.cpp                    # Entry point
├── EventLoop.cpp/hpp           # Event loop and poll()
├── Connection.cpp/hpp          # HTTP connection handler
├── ConnectionManager.cpp/hpp   # Connection lifecycle
├── HttpRequest.cpp/hpp         # Request representation
├── HttpRequestParser.cpp/hpp   # Request parsing
├── Response.cpp/hpp            # Response generation
├── CgiHandler.cpp/hpp          # CGI execution
├── ConfigLoader.cpp/hpp        # Config parser
├── ServerConfig.cpp/hpp        # Server configuration
├── LocationConfig.cpp/hpp      # Location blocks
├── Listener.cpp/hpp            # Listening sockets
├── ListenerRegistry.cpp/hpp    # Listener management
└── SignalHandler.cpp/hpp       # Signal handling
```

## Standards

This implementation follows:
- [RFC 9112 - HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc9112)
- [RFC Editor Standards](https://www.rfc-editor.org/about/)
- [IBM CGI Specification](https://www.ibm.com/docs/en/i/7.6.0?topic=functionality-cgi)

## 42 Requirements

- C++98 standard
- No `execve()` except for CGI
- Non-blocking at all times
- Single `poll()` in main loop
- All FD operations through `poll()`
- No `errno` after socket operations
- Must never crash

## Authors

me and [@PyrrhaSchnee](https://github.com/PyrrhaSchnee)

## License

This project is part of the 42 school curriculum.

## Acknowledgments

- 42 School for the project subject
- NGINX for behavioral reference
- The HTTP/1.1 specification authors
