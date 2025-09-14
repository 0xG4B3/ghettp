#include "../include/ghettp.hpp"
#include <iostream>
#include <sstream>

namespace ghettp {

server::server(int port) : m_socket(port) {
    m_socket.setRequestHandler([this](const HttpRequest& req) {
        return routeRequest(req);
    });
}

server::~server() {
    stop();
}

void server::get(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
    m_routes["GET"][path] = handler;
}

void server::post(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
    m_routes["POST"][path] = handler;
}

void server::put(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
    m_routes["PUT"][path] = handler;
}

void server::del(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler) {
    m_routes["DELETE"][path] = handler;
}

HttpResponse server::html(const std::string& content, int status_code) {
    HttpResponse response;
    response.status_code = status_code;
    response.status_text = (status_code == 200) ? "OK" : "Error";
    response.headers["Content-Type"] = "text/html";
    response.body = content;
    return response;
}

HttpResponse server::json(const std::string& content, int status_code) {
    HttpResponse response;
    response.status_code = status_code;
    response.status_text = (status_code == 200) ? "OK" : "Error";
    response.headers["Content-Type"] = "application/json";
    response.body = content;
    return response;
}

HttpResponse server::text(const std::string& content, int status_code) {
    HttpResponse response;
    response.status_code = status_code;
    response.status_text = (status_code == 200) ? "OK" : "Error";
    response.headers["Content-Type"] = "text/plain";
    response.body = content;
    return response;
}

void server::start() {
    m_running = true;
    m_server_thread = std::thread([this]() {
        m_socket.run();
    });
}

void server::stop() {
    if (m_running) {
        m_running = false;
        m_socket.stop();
        if (m_server_thread.joinable()) {
            m_server_thread.join();
        }
    }
}

HttpResponse server::routeRequest(const HttpRequest& request) {
    auto method_routes = m_routes.find(request.method);
    if (method_routes != m_routes.end()) {
        auto route = method_routes->second.find(request.path);
        if (route != method_routes->second.end()) {
            return route->second(request);
        }
    }

    HttpResponse response;
    response.status_code = 404;
    response.status_text = "Not Found";
    response.headers["Content-Type"] = "text/html";
    response.body = "<html><body><h1>404 - Not Found</h1></body></html>";
    return response;
}

}
