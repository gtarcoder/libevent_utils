#include"tcp_server.h"
#include"connection.h"
#include<string>
#include "common_defs.h"
using namespace std;

//functions to be used as function pointer in libevent
void TcpServer::ListenCallback(struct evconnlistener* listener, evutil_socket_t sock, struct sockaddr* addr, int socklen, void* arg){
    TcpServer* server = (TcpServer*)arg;
    server->OnListen(listener, sock, addr, socklen);
}

void TcpServer::InnerConnectionReadCallback(struct bufferevent* event, void* arg){
    ConnEventArg* p = (ConnEventArg*)arg;
    p->server->OnInnerConnectionRead(event, p->conn);
}

void TcpServer::InnerConnectionEventCallback(struct bufferevent* event, short events, void* arg){
    ConnEventArg* p = (ConnEventArg*)arg;
    p->server->OnInnerConnectionEvent(event, p->conn, events);
}

TcpServer::TcpServer():
listener_(NULL)
,listen_fd_(-1)
,next_client_index_(0){
}

TcpServer::~TcpServer(){
    if(listener_)
        evconnlistener_free(listener_);
}

void TcpServer::Bind(const char* ip, uint16_t port, struct event_base* base){
    base_ = base;
    listen_addr_.sin_family = AF_INET;
    listen_addr_.sin_addr.s_addr = inet_addr(ip);
    listen_addr_.sin_port = htons(port);
    listener_ = evconnlistener_new_bind(
                base_, 
                ListenCallback,
                this, 
                LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,
                -1,
                (const sockaddr*)&listen_addr_, 
                sizeof(listen_addr_));
    if (!listener_){
        printf("evconnlistener create failed in %s:%s!\n", __FILE__, __func__);
    }
}

void TcpServer::OnListen(struct evconnlistener* listener, evutil_socket_t sock, struct sockaddr* addr, int socklen){
    string client_ip = string(inet_ntoa(((struct sockaddr_in*)addr)->sin_addr));
    uint16_t client_port = ntohs(((struct sockaddr_in*)addr)->sin_port);
    printf("receive connection from %s:%d\n", client_ip.c_str(), client_port);
    Connection* conn = new Connection(client_ip, client_port, 0, next_client_index_);
    connections_[next_client_index_++] = conn;
    conn->Initialize(base_, sock);
}

void TcpServer::OnInnerConnectionRead(bufferevent* event, Connection* conn){
}

void TcpServer::OnInnerConnectionEvent(bufferevent* event, Connection* conn, short events){
} 
