#include "../include/socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <cstring>

namespace ghettp {

socket::socket(int port) : m_port(port), m_socket_fd(-1) {
    m_socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket_fd == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to configure socket");
    }

    memset(&m_address, 0, sizeof(m_address));
    m_address.sin_family = AF_INET;
    m_address.sin_addr.s_addr = INADDR_ANY;
    m_address.sin_port = htons(m_port);

    if (bind(m_socket_fd, (struct sockaddr*)&m_address, sizeof(m_address)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(m_socket_fd, 5) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    m_request_handler = [](const HttpRequest& req) -> HttpResponse {
        HttpResponse response;
        response.status_code = 404;
        response.status_text = "Not Found";
        response.body = "<html><body><h1>404 - Not Found</h1></body></html>";
        response.headers["Content-Type"] = "text/html";
        return response;
    };
}

socket::~socket() {
    stop();
    if (m_socket_fd != -1) {
        close(m_socket_fd);
    }
}

void socket::setRequestHandler(RequestHandler handler) {
    m_request_handler = handler;
}

void socket::run() {
    m_running = true;
    std::cout << "Server running on port " << m_port << std::endl;

    while (m_running) {
        socklen_t addr_len = sizeof(m_address);
        int client_socket = accept(m_socket_fd, (struct sockaddr*)&m_address, &addr_len);
        if (client_socket < 0) {
            if (m_running) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }

        std::thread client_thread(&socket::handleClient, this, client_socket);
        client_thread.detach();
    }
}

void socket::stop() {
    m_running = false;
    if (m_socket_fd != -1) {
        shutdown(m_socket_fd, SHUT_RDWR);
    }
}

void socket::handleClient(int client_socket) {
    char buffer[4096] = {0};
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    try {
        std::string raw_request(buffer);
        HttpRequest request = parseRequest(raw_request);
        HttpResponse response = m_request_handler(request);
        std::string response_str = buildResponse(response);
        send(client_socket, response_str.c_str(), response_str.length(), 0);
    } catch (const std::exception& e) {
        std::string error_response =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 21\r\n"
            "\r\n"
            "Internal Server Error";
        send(client_socket, error_response.c_str(), error_response.length(), 0);
    }

    close(client_socket);
}

HttpRequest socket::parseRequest(const std::string& raw_request) {
    HttpRequest request;
    std::istringstream stream(raw_request);
    std::string line;

    if (std::getline(stream, line)) {
        if (line.back() == '\r') {
            line.pop_back();
        }
        std::istringstream first_line(line);
        first_line >> request.method >> request.path >> request.version;
    }

    while (std::getline(stream, line) && line != "\r") {
        if (line.back() == '\r') {
            line.pop_back();
        }
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            if (!value.empty() && value[0] == ' ') {
                value = value.substr(1);
            }
            request.headers[key] = value;
        }
    }

    std::string body_line;
    while (std::getline(stream, body_line)) {
        request.body += body_line + "\n";
    }
    if (!request.body.empty()) {
        request.body.pop_back();
    }

    return request;
}

std::string socket::buildResponse(const HttpResponse& response) {
    std::ostringstream response_stream;

    response_stream << "HTTP/1.1 " << response.status_code << " " << response.status_text << "\r\n";

    for (const auto& header : response.headers) {
        response_stream << header.first << ": " << header.second << "\r\n";
    }

    response_stream << "Content-Length: " << response.body.length() << "\r\n";
    response_stream << "\r\n";
    response_stream << response.body;

    return response_stream.str();
}

}
