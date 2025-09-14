# GHETTP - Lightweight C++ HTTP Server Library

![C++](https://img.shields.io/badge/C%2B%2B-17%2B-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-green.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)

A modern, lightweight, and easy-to-use HTTP server library written in C++17. GHETTP provides a simple API for creating REST APIs and web servers with minimal boilerplate code.

## Features

- **Lightweight**: Minimal dependencies, built with standard C++ libraries
- **Easy to Use**: Simple and intuitive API inspired by modern web frameworks
- **HTTP/1.1 Support**: Full HTTP/1.1 protocol implementation
- **RESTful**: Support for GET, POST, PUT, DELETE methods
- **Multiple Response Types**: Built-in support for HTML, JSON, and plain text responses
- **Multi-threaded**: Each client connection handled in a separate thread
- **Thread-safe**: Uses modern C++ concurrency primitives (`std::atomic`)
- **Signal Handling**: Graceful shutdown with SIGINT/SIGTERM handling
- **Route-based**: Clean URL routing system

## Technical Specifications

### Requirements

- **C++ Standard**: C++17 or higher
- **Platform**: Linux (uses POSIX sockets)
- **Compiler**: GCC 7+, Clang 6+, or equivalent
- **Dependencies**: 
  - pthread (POSIX Threads)
  - Standard C++ libraries

### Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     Client      │◄──►│   GeHTTP Server │◄──►│  User Handlers  │
│   (Browser/App) │    │    (Library)    │    │   (Your Code)   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │ Socket Layer    │
                    │ (TCP/IP)        │
                    └─────────────────┘
```

### Core Components

- **`ghettp::server`**: Main server class for handling HTTP requests
- **`ghettp::socket`**: Low-level socket management and connection handling
- **`HttpRequest`**: Request data structure (method, path, headers, body)
- **`HttpResponse`**: Response data structure (status, headers, body)
- **`RequestHandler`**: Function type for handling requests

## Installation

### Building from Source

```bash
# Clone the repository
git clone <repository-url>
cd ghettp

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Optional: Run tests
./ghettp_example
```

### CMake Integration

Add GeHTTP to your project using CMake:

```cmake
# Add GeHTTP as subdirectory
add_subdirectory(path/to/ghettp)

# Link to your target
target_link_libraries(your_target ghettp)
```

## Quick Start

### Basic Server

```cpp
#include "ghettp.hpp"
#include <iostream>

int main() {
    // Create server on port 8080
    ghettp::server app(8080);
    
    // Define a simple route
    app.get("/", [](const ghettp::HttpRequest& req) {
        return ghettp::server::html("<h1>Hello, World!</h1>");
    });
    
    // Start the server
    std::cout << "Server running on http://localhost:8080" << std::endl;
    app.start();
    
    // Keep running
    std::cin.get();
    
    return 0;
}
```

### RESTful API Example

```cpp
#include "ghettp.hpp"
#include <iostream>
#include <json/json.h> // Example with nlohmann/json

int main() {
    ghettp::server app(8080);
    
    // GET endpoint
    app.get("/api/users", [](const ghettp::HttpRequest& req) {
        std::string json = R"({
            "users": [
                {"id": 1, "name": "John Doe"},
                {"id": 2, "name": "Jane Smith"}
            ]
        })";
        return ghettp::server::json(json);
    });
    
    // POST endpoint
    app.post("/api/users", [](const ghettp::HttpRequest& req) {
        // Process req.body (JSON data)
        std::string response = R"({"message": "User created", "id": 3})";
        return ghettp::server::json(response, 201);
    });
    
    // Dynamic route with parameters
    app.get("/api/users", [](const ghettp::HttpRequest& req) {
        // Extract ID from request path
        // Implementation depends on your routing needs
        return ghettp::server::json(R"({"id": 1, "name": "John Doe"})");
    });
    
    app.start();
    std::cin.get();
    return 0;
}
```

## API Reference

### Server Class

#### Constructor
```cpp
explicit server(int port);
```
Creates a new HTTP server listening on the specified port.

#### HTTP Methods
```cpp
void get(const std::string& path, RequestHandler handler);
void post(const std::string& path, RequestHandler handler);
void put(const std::string& path, RequestHandler handler);
void del(const std::string& path, RequestHandler handler);
```
Register handlers for different HTTP methods.

#### Response Helpers
```cpp
static HttpResponse html(const std::string& content, int status_code = 200);
static HttpResponse json(const std::string& content, int status_code = 200);
static HttpResponse text(const std::string& content, int status_code = 200);
```
Create responses with appropriate Content-Type headers.

#### Server Control
```cpp
void start();  // Start the server (non-blocking)
void stop();   // Stop the server gracefully
```

### Data Structures

#### HttpRequest
```cpp
struct HttpRequest {
    std::string method;      // GET, POST, PUT, DELETE, etc.
    std::string path;        // URL path
    std::string version;     // HTTP version
    std::map<std::string, std::string> headers;  // HTTP headers
    std::string body;        // Request body
};
```

#### HttpResponse
```cpp
struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string, std::string> headers;
    std::string body;
};
```

## Advanced Usage

### Custom Response Headers

```cpp
app.get("/api/data", [](const ghettp::HttpRequest& req) {
    ghettp::HttpResponse response;
    response.status_code = 200;
    response.status_text = "OK";
    response.headers["Content-Type"] = "application/json";
    response.headers["Access-Control-Allow-Origin"] = "*";
    response.headers["Cache-Control"] = "no-cache";
    response.body = R"({"data": "custom response"})";
    return response;
});
```

### Error Handling

```cpp
app.get("/api/error-example", [](const ghettp::HttpRequest& req) {
    try {
        // Your logic here
        return ghettp::server::json(R"({"success": true})");
    } catch (const std::exception& e) {
        return ghettp::server::json(
            R"({"error": ")" + std::string(e.what()) + R"("})", 
            500
        );
    }
});
```

### Signal Handling for Graceful Shutdown

```cpp
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};
ghettp::server* server_ptr = nullptr;

void signalHandler(int signal) {
    std::cout << "Shutting down..." << std::endl;
    running = false;
    if (server_ptr) {
        server_ptr->stop();
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    ghettp::server app(8080);
    server_ptr = &app;
    
    // ... register routes ...
    
    app.start();
    
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}
```

## Performance Considerations

- **Thread Pool**: Each client connection creates a new thread. Consider implementing a thread pool for high-concurrency scenarios.
- **Memory Usage**: Request/response data is copied. For large payloads, consider streaming implementations.
- **Keep-Alive**: Currently, connections are closed after each request. HTTP keep-alive could improve performance.

## Examples

The `example/` directory contains a complete working example:

```bash
cd build
./ghettp_example
```

