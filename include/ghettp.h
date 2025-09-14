#pragma once
namespace ghettp {
    class server {

    public:
        server(const int& port);
        ~server();

    private:
        const int& port;

    };
}