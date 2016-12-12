#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdio.h>
#include <string>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
class Connection{
friend class TcpServer;
public:
    Connection(std::string peer_ip, uint16_t peer_port, uint16_t local_port = 0, uint16_t index = 0);
    virtual ~Connection();

    virtual void OnPeerRead();
    virtual void OnPeerWrite();
    virtual void OnPeerEvent(short events);

    virtual void OnInnerConnectionRead(struct bufferevent*);
    virtual void OnInnerConnectionEvent(struct bufferevent*, short events);

    virtual bool Start();

    void Initialize(struct event_base* base, evutil_socket_t fd=-1, bool write_callback_on=false);
    //do not add 4-bytes-leng HEAD
    void WriteRaw(char* buffer, int len);
    //add 4-bytes-len HEAD
    void Write(char* buffer, int len);
    inline uint16_t GetIndex(){ return index_; };
    inline void SetEventPriority(int priority){ priority_ = priority; };
    inline uint16_t GetPeerPort(){ return peer_port_; };
    inline uint16_t GetLocalPort(){ return local_port_; };
protected:
    static void InnerConnectionReadCallback(struct bufferevent* , void*);
    static void InnerConnectionEventCallback(struct bufferevent* , short events, void*);
    static void PeerReadCallback(struct bufferevent*, void* arg);
    static void PeerWriteCallback(struct bufferevent*, void* arg);
    static void PeerEventCallback(struct bufferevent*, short events, void*);

    struct bufferevent* peer_event_; 
    struct event_base* base_;
    //for inner communication, when the listen connection process and connection's data process are in different event_base loop(in different thread)
    //socket pair, used to send signal from server to conn
    struct bufferevent* conn_2_server_pair_[2];
    //socket pair, used to send signal from conn to server
    struct bufferevent* server_2_conn_pair_[2];

    int          fd_;
    std::string peer_ip_;
    uint16_t    peer_port_;
    uint16_t    local_port_;
    uint16_t    index_;
    int         priority_;
    bool        connected_;
    char        buffer_[0xffff];
    int         buffer_len_;
};

#endif

