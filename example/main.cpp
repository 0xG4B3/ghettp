#include "../include/ghettp.hpp"
#include <iostream>
#include <csignal>
#include <atomic>

using namespace ghettp;

std::atomic<bool> running{true};

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
    running = false;
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        server app(8080);

        app.get("/", [](const HttpRequest& req) {
            std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>GeHTTP Server</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .container { max-width: 600px; margin: 0 auto; }
        .endpoint { background: #f5f5f5; padding: 10px; margin: 10px 0; border-radius: 5px; }
    </style>
</head>
<body>
    <div class="container">
        <p>Server is running successfully!</p>
        <h2>Available Endpoints:</h2>
        <div class="endpoint"><strong>GET /</strong> - This page</div>
        <div class="endpoint"><strong>GET /api/status</strong> - Server status (JSON)</div>
        <div class="endpoint"><strong>GET /api/time</strong> - Current time (JSON)</div>
        <div class="endpoint"><strong>POST /api/echo</strong> - Echo request data</div>
        <div class="endpoint"><strong>GET /hello/:name</strong> - Personalized greeting</div>
    </div>
</body>
</html>
            )";
            return server::html(html);
        });

        app.get("/api/status", [](const HttpRequest& req) {
            return server::json(R"({"status": "running", "server": "GeHTTP"})");
        });

        app.get("/api/time", [](const HttpRequest& req) {
            auto now = std::time(nullptr);
            return server::json("{\"timestamp\": " + std::to_string(now) + "}");
        });

        app.post("/api/echo", [](const HttpRequest& req) {
            std::string response = "{\"method\": \"" + req.method +
                                 "\", \"path\": \"" + req.path +
                                 "\", \"body\": \"" + req.body + "\"}";
            return server::json(response);
        });

        app.get("/hello", [](const HttpRequest& req) {
            std::string name = "World";
            auto query_pos = req.path.find('?');
            if (query_pos != std::string::npos) {
                std::string query = req.path.substr(query_pos + 1);
                auto name_pos = query.find("name=");
                if (name_pos != std::string::npos) {
                    name = query.substr(name_pos + 5);
                }
            }

            std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Hello )" + name + R"(</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 100px; }
        h1 { color: #333; }
    </style>
</head>
<body>
    <h1>Hello, )" + name + R"(!</h1>
    <p><a href="/">← Back to home</a></p>
</body>
</html>
            )";
            return server::html(html);
        });
        std::cout << "Press Ctrl+C to stop the server" << std::endl;

        app.start();

        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "Stopping server..." << std::endl;
        app.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Server stopped successfully" << std::endl;
    return 0;
}
