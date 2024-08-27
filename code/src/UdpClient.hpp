#include <IPAddress.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

class LightUDP {
   private:
    int udp_server;
    IPAddress remote_ip;
    uint32_t remote_ip_int;
    uint16_t server_port;
    uint16_t remote_port;
    /**
     * 接收 的最后一条消息的地址
     * */
    IPAddress rec_remote_ip;
    uint16_t rec_remote_port;
   public:
    LightUDP();
    void begin(IPAddress address, uint16_t port);
    void setServer(const char* host, uint16_t port);
    void send(uint8_t* buf, size_t len);
    void stop();

    int recv(uint8_t* buf, size_t size);

    const IPAddress &getRecRemoteIp() const;

    uint16_t getRecRemotePort() const;
};