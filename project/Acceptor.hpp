#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cerrno>


class Acceptor {
private:
    int sock = -1;

    const int DOMAIN;
    const int SOCK_TYPE;
    const int PORT;
    const char* ADDR;

    const int listen_count;


    void create_sock() {
        sock = socket(DOMAIN, SOCK_TYPE, 0);

        if (sock == -1) {
            std::cerr << "Error in create socket: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
    }

    void non_blocking_mode() {
        int flag = fcntl(sock, F_GETFL, 0);
        
        if (flag == -1) {
            std::cerr << "Error in read fd: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }

        if (fcntl(sock, F_SETFL, flag | O_NONBLOCK) == -1) {
            std::cerr << "Error in set non block mode: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
    }

    void bind_socket() {
        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = DOMAIN;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr(ADDR);

        socklen_t server_len = sizeof(server_addr); 

        int bind_result = bind(sock, (struct sockaddr*) &server_addr, server_len);

        if (bind_result == -1) {
            std::cerr << "Error in bind socket: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
    }

    void listening() {
        int listen_res = listen(sock, listen_count);

        if (listen_res == -1) {
            std::cerr << "Error in lestening: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
    }


public:

    int client_accept() {
        /* в случае ошибки - возвращаеться -1*/
        struct sockaddr_in client_addr;    
        socklen_t len_client_addr= sizeof(client_addr);

        int client_fd = accept(sock, (struct sockaddr*)&client_addr, &len_client_addr);

        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return -1;
            }
            std::cerr << "Error in client accept: " << strerror(errno) << "\n";
            return -1;
        } 

        int flags = fcntl(client_fd, F_GETFL, 0);
            
        if (flags == -1) {
            std::cerr << "Error getting client socket flags: " << strerror(errno) << "\n";
            close(client_fd);
            return - 1;
        }
        
        if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            std::cerr << "Error setting client non-blocking: " << strerror(errno) << "\n";
            close(client_fd);
            return -1;
        }  

        return client_fd;  
    }

    explicit Acceptor (
        int domain = AF_INET,
        int sock_type = SOCK_STREAM,
        int port = 8080, 
        const char* addr = "127.0.0.1", 
        int listen_count = 10
    ) :

    DOMAIN(domain), SOCK_TYPE(sock_type), PORT(port), ADDR(addr), listen_count(listen_count){
        create_sock();
        non_blocking_mode();
        bind_socket();
        listening();
    }

    ~ Acceptor () {
        if (sock != -1) {
            close(sock);
        }
    }

};