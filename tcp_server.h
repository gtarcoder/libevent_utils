#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_
#include<stdint.h>
#include<vector>
#include<unordered_map>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<event2/listener.h>
class Connection;
class TcpServer{
public:
    TcpServer();
    virtual ~TcpServer();

    virtual void OnListen(struct evconnlistener*, evutil_socket_t sock, struct sockaddr* addr, int socklen);
    virtual void OnInnerConnectionRead(struct bufferevent*, Connection*);
    virtual void OnInnerConnectionEvent(struct bufferevent*, Connection*, short events);

    void Bind(const char* ip, uint16_t port, struct event_base* = NULL );
    //in my implementation, the event_base handling new connection from client is
    //in different thread with the event_base which will handle connection's data receiving and sending
    //SO, use the socketpair to realize the communication between different threads
    void NotifyAllConnections(const char* buf, int len);
    void NofityConnection(uint16_t connection_index, const char* buf, int len); 

    inline int GetListenFd(){
        return listen_fd_;
    }

protected:
    static void InnerConnectionReadCallback(struct bufferevent* , void*);
    static void InnerConnectionEventCallback(struct bufferevent* , short events, void*);
    static void ListenCallback(struct evconnlistener*, evutil_socket_t sock, struct sockaddr* addr, int socklen, void* arg);

    struct event_base* base_;
    struct evconnlistener* listener_;
    struct sockaddr_in listen_addr_;
    int listen_fd_;
    std::unordered_map<uint16_t, Connection*> connections_;
    uint16_t next_client_index_;
};
#endif
