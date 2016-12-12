#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_
#include"connection.h"
class TcpClient:public Connection{
public:
    TcpClient(std::string peer_ip, uint16_t peer_port, uint16_t local_port = 0, uint16_t index = 0);
    virtual ~TcpClient();

    void OnPeerRead();
    void OnPeerEvent(short events);

    bool Start();
private:
    bool DoConnect();
    static void KeepAliveCallback(evutil_socket_t fd, short event, void* arg);
    struct event* keepalive_event_;
};
#endif
