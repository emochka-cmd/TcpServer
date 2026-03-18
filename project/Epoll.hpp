#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>


class Epoll {
private:    
    int epoll_sock = -1;

    void create_epoll() {
        epoll_sock = epoll_create1(0);

        if (epoll_sock == -1) {
            std::cerr << "Error in create epoll: " << strerror(errno) << "\n";
        }
    }
public:
    void epoll_add(const int& fd) {
        struct epoll_event event;
        std::memset(&event, 0, sizeof(event));

        event.events = EPOLLIN | EPOLLET;
        event.data.fd = fd;
        
        int add_res = epoll_ctl(epoll_sock, EPOLL_CTL_ADD, fd, &event);

        if (add_res == -1) {
            std::cerr << "Error in epoll ctl add: " << strerror(errno) << "\n";
        }
    }

    void epoll_mod(const int& fd, const uint32_t& EVENT) {
        struct epoll_event event;
        std::memset(&event, 0, sizeof(event));

        event.events = EVENT;
        event.data.fd = fd;

        int mod_res =  epoll_ctl(epoll_sock, EPOLL_CTL_MOD, fd, &event);

        if (mod_res == -1) {
            std::cerr << "Error in epoll ctl mod: " << strerror(errno) << "\n";
        }
    }



    explicit Epoll (int server_fd) {
        create_epoll();
        epoll_add(server_fd);
    }

    ~Epoll () {
        if (epoll_sock != -1) {
            close(epoll_sock);
        }
    }

};