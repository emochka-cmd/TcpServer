#include <cstdint>
#include <string>
#include <unordered_map>



class ClientManager {
    struct Client {
        uint16_t message_len;
        std::string buffer;
    };

    // client sock + Client
    std::unordered_map<int, Client> clients;

};