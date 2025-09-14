#pragma once

#include "socket.hpp"
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <atomic>

namespace ghettp {

using HttpRequest = ghettp::HttpRequest;
using HttpResponse = ghettp::HttpResponse;
using RequestHandler = ghettp::RequestHandler;

class server {
private:
    socket m_socket;
    std::map<std::string, std::map<std::string, RequestHandler>> m_routes;
    std::thread m_server_thread;
    std::atomic<bool> m_running{false};

    HttpResponse routeRequest(const HttpRequest& request);

public:
    explicit server(int port);
    ~server();

    void get(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler);
    void post(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler);
    void put(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler);
    void del(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler);

    static HttpResponse html(const std::string& content, int status_code = 200);
    static HttpResponse json(const std::string& content, int status_code = 200);
    static HttpResponse text(const std::string& content, int status_code = 200);

    void start();
    void stop();
};

}
