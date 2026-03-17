#include <fcntl.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


class Client {
private:
    int sock = -1;

    const int DOMAIN; 
    const int PORT;
    const char* ADDR;


    void creat_sock() {
        sock = socket(AF_INET, SOCK_STREAM, 0); 
        
        if (sock == -1) {
            std::cerr << "client socket error: " << strerror(errno) << "\n";
        }
    }

    void connect_to_server() {
        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = DOMAIN; 
        server_addr.sin_port = htons(PORT); 

        if (inet_pton(DOMAIN, ADDR, &server_addr.sin_addr) <= 0) {
            std::cerr << "invalid server addr" << strerror(errno) << "\n";
            close(sock);
            sock = -1;
            return;
        }

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            std::cerr << "connect error: " << strerror(errno) << "\n";
            close(sock);
            sock = -1;
            return;
        }
    }

    
public:
    void send_message(const std::string& message) {
        if (sock == -1) {
            std::cerr << "Error - sending is not possible. Sock == -1" << "\n";
            return;
        }

        static const uint16_t MAX_LENGTH = 65535;
        
        if (message.length() > MAX_LENGTH) {
            std::cerr << "Message from user " << sock << " to big. Max - " << MAX_LENGTH << " bytes";
            return;
        }
        
        uint16_t len_message = static_cast<uint16_t>((message.length())); 
        uint16_t net_len = htons(len_message);
    
        struct iovec iov[2];

        iov[0].iov_base = &net_len;
        iov[0].iov_len = sizeof(net_len);


        iov[1].iov_base = const_cast<char*>(message.data());
        iov[1].iov_len = message.length();


        size_t total_to_send = sizeof(net_len) + message.length();
        size_t already_send = 0;


        while (already_send < total_to_send) {
            ssize_t curr_sent = writev(sock, iov, 2);

            if (curr_sent > 0) {
                already_send += curr_sent;
                
               
                while (curr_sent > 0) {
                    if (iov[0].iov_len > 0) {
                        if (static_cast<size_t>(curr_sent) >= iov[0].iov_len) {
                            curr_sent -= iov[0].iov_len;
                            iov[0].iov_len = 0;
                            iov[0].iov_base = nullptr;
                        }

                        else {
                            iov[0].iov_len -= curr_sent;
                            iov[0].iov_base = static_cast<char*>(iov[0].iov_base) + curr_sent; 
                            curr_sent = 0;
                        }
                    }

                    else if (iov[1].iov_len > 0) {
                        if (static_cast<size_t>(curr_sent) >= iov[1].iov_len) {
                            curr_sent -= iov[1].iov_len;
                            iov[1].iov_len = 0;
                            iov[1].iov_base = nullptr;
                        }

                        else {
                            iov[1].iov_len -= curr_sent;
                            iov[1].iov_base = static_cast<char*>(iov[1].iov_base) + curr_sent; 
                            curr_sent = 0;
                        }
                    }
                }
                
                continue;
            }

            if (curr_sent == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // в будующем переделать для производительности
                    continue;
                }
            }

            std::cerr << "Send error: " << strerror(errno) << "\n";
            return; 
        }
    }    
    
    explicit Client (int DOMAIN = AF_INET, int PORT = 8080, const char* ADDR = "127.0.0.1") :
    DOMAIN(DOMAIN), PORT(PORT), ADDR(ADDR) {
        creat_sock();
        
        if (sock != -1) {
            connect_to_server();
        }
    }

    ~Client () {
        if (sock != -1) {
            close(sock);
        }
    }
};