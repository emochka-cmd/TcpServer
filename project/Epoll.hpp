#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>


class Epoll {
private:    
    int epoll_sock = -1;

    static constexpr int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];


    void create_epoll() {
        epoll_sock = epoll_create1(0);

        if (epoll_sock == -1) {
            std::cerr << "Error in create epoll: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
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

    void epoll_del (const int& fd) {
        if (epoll_ctl(epoll_sock, EPOLL_CTL_DEL, fd, NULL) == -1) {
            std::cerr << "Error in epoll ctl del: " << strerror(errno) << "\n";
        }
    }

    int wait(int timeout = -1) {
        int wait_res = epoll_wait(epoll_sock, events, MAX_EVENTS, timeout);
        if (wait_res == -1 && errno != EINTR) {
            std::cerr << "Error in epoll wait: " << strerror(errno) << "\n";
        }
        return wait_res;
    }

    const epoll_event& get_event(int i) const {
        return events[i];
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