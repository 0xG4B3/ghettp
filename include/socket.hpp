#pragma once

#include <string>
#include <functional>
#include <map>
#include <netinet/in.h>
#include <atomic>

namespace ghettp {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string, std::string> headers;
    std::string body;
};

using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

class socket {
private:
    int m_port;
    int m_socket_fd;
    sockaddr_in m_address;
    socklen_t m_addressLength = sizeof(m_address);
    std::atomic<bool> m_running{false};
    RequestHandler m_request_handler;

    void handleClient(int client_socket);
    HttpRequest parseRequest(const std::string& raw_request);
    std::string buildResponse(const HttpResponse& response);

public:
    explicit socket(int port);
    ~socket();
    void setRequestHandler(RequestHandler handler);
    void run();
    void stop();
};

}
