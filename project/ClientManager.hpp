#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unordered_map>
#include <vector>



class ClientManager {
private:
    enum class RecvState {
        READ_LEN,
        READ_BODY
    };

    struct Client {
        RecvState recv_state = RecvState::READ_LEN;
        uint16_t expected_len = 0;
        uint16_t recived_bytes = 0;
        std::vector<char> buffer;
    };

    // client sock + Client
    std::unordered_map<int, Client> clients;

public:
    bool read_from_client(int& fd) {
        // if any error - return false
        auto it = clients.find(fd);
        if (it == clients.end()) return false;

        Client& client = it -> second;

        while (true) {
            if (client.recv_state == RecvState::READ_LEN) {
                size_t to_read = sizeof(uint16_t) - client.recived_bytes;
                ssize_t res = recv(fd, reinterpret_cast<char*>(&client.expected_len) + client.recived_bytes, to_read, 0);

                if (res > 0) {
                    client.recived_bytes += res;
                    if (client.recived_bytes == sizeof(uint16_t)) {
                        client.expected_len = ntohs(client.expected_len);
                        client.buffer.resize(client.expected_len);
                        client.recived_bytes = 0;
                        client.recv_state = RecvState::READ_BODY;
                        
                        if (client.expected_len == 0) return true; 
                    }
                }

                else if (res == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        return false;
                    }
                    return false;
                }

                else return false;
            }

            if (client.recv_state == RecvState::READ_BODY) {
                size_t to_read = client.expected_len - client.recived_bytes;
                ssize_t res = recv(fd, client.buffer.data() + client.recived_bytes, to_read, 0);
                
                if (res > 0) {
                    client.recived_bytes += res;
                    if (client.recived_bytes == client.expected_len) {
                        client.recv_state = RecvState::READ_LEN;
                        client.recived_bytes = 0;
                        return true;
                    }
                }

                else if (res == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        return false;
                    }
                    return false;
                }

                else return false;
            }
        }
    }


    ClientManager () {

    }

    ~ClientManager () {

    }

};