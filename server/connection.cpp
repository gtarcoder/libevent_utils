#include <assert.h>
#include "connection.h"
#include "utils.h"
#include "common_defs.h"
void Connection::InnerConnectionReadCallback(struct bufferevent* event, void* arg){
    ConnEventArg* p = (ConnEventArg*)arg;
    p->conn->OnInnerConnectionRead(event);
}

void Connection::InnerConnectionEventCallback(struct bufferevent* event, short events, void* arg){
    ConnEventArg* p = (ConnEventArg*)arg;
    p->conn->OnInnerConnectionEvent(event, events);
}

void Connection::PeerReadCallback(struct bufferevent* bev, void* arg){
    Connection* item = (Connection*)arg;
    struct evbuffer* input = bufferevent_get_input(bev);
    if(item->buffer_len_ < 0){
        evbuffer_remove(input, (void*)&item->buffer_len_, 4);   
        item->buffer_len_ = ntohl(item->buffer_len_);
    }
    if(item->buffer_len_ > 0 && item->buffer_len_ <= evbuffer_get_length(input)){
        evbuffer_remove(input, (void*)item->buffer_, item->buffer_len_); 
        item->buffer_[item->buffer_len_] = '\0';
        //process the data received
        item->OnPeerRead();
        //cleared the buffer
        item->buffer_len_ = -1;
    }
};
void Connection::PeerWriteCallback(struct bufferevent* bev, void* arg){
    Connection* item = (Connection*)arg;
    item->OnPeerWrite();
};
void Connection::PeerEventCallback(struct bufferevent* bev, short events, void* arg){
    Connection* item = (Connection*)arg;
    item->OnPeerEvent(events);
};

Connection:: Connection(
    std::string peer_ip
    ,uint16_t peer_port
    ,uint16_t local_port
    ,uint16_t index 
    ):
    peer_ip_(peer_ip)
    ,peer_port_(peer_port)
    ,local_port_(local_port)
    ,index_(index)
    ,fd_(-1)
    ,connected_(false)
    ,buffer_len_(-1)
{
  printf("local port : %u\n", local_port_);
  printf("peer port : %u\n", peer_port_);
  peer_event_ = NULL;
  conn_2_server_pair_[0] = conn_2_server_pair_[1] = NULL;
  server_2_conn_pair_[0] = server_2_conn_pair_[1] = NULL;
}

Connection:: ~Connection(){
  printf("connection destroyed\n");
  bufferevent_free(peer_event_);
  if(conn_2_server_pair_[0]){
    bufferevent_free(conn_2_server_pair_[0]);
    bufferevent_free(conn_2_server_pair_[1]);
  }
  if(server_2_conn_pair_[0]){
    bufferevent_free(server_2_conn_pair_[0]);
    bufferevent_free(server_2_conn_pair_[1]);
  }
}

bool Connection::Start(){
    if (SetKeepLive(fd_, 20, 5, 3) < 0)
       return false;
    if (SetRecvSendBufSize(fd_, 0xfffff) < 0)
       return false;
    if(SetReuseAddr(fd_) < 0)
       return false;
    return true;
}

void Connection::Initialize(struct event_base* base, evutil_socket_t fd, bool write_callback_on){
    fd_ = fd;
    base_ = base;
    peer_event_ = bufferevent_socket_new(base, fd_, BEV_OPT_CLOSE_ON_FREE);
    if(!peer_event_){
        printf("create bufferevent failed in %s:%s!\n", __FILE__, __func__);
        fd_ = -1;
        return;
    }
    bufferevent_setcb(peer_event_, PeerReadCallback, write_callback_on? PeerWriteCallback: NULL, PeerEventCallback, this);
    bufferevent_enable(peer_event_, EV_WRITE|EV_READ);
    //for TCP packet, we have 4 bytes-len head
    bufferevent_setwatermark(peer_event_, EV_READ, 4, 0xfffff);
}

void Connection::Write(char* buffer, int len){
    int packet_len = htonl(len);
    bufferevent_write(peer_event_, &packet_len, 4);
    bufferevent_write(peer_event_, buffer, len);
}

void Connection::WriteRaw(char* buffer, int len){
    bufferevent_write(peer_event_, buffer, len); 
}

void Connection::OnPeerRead(){
    //this is for test
    printf("received data from peer, length = %d, data = %s\n", buffer_len_, buffer_);
    usleep(1000000);
    
    char *msg = "hello client!\n";
    Write(msg, strlen(msg));
}

void Connection::OnPeerWrite(){
}

void Connection::OnPeerEvent(short events){
   if (events & BEV_EVENT_CONNECTED){
        connected_ = true;
        printf("connected!\n");
   }else if (events & BEV_EVENT_EOF){
        evutil_closesocket(fd_);
        fd_ = -1;
        bufferevent_setfd(peer_event_, -1);
        connected_ = false;
        printf("connection closed!\n");
   }else if (events & BEV_EVENT_ERROR){
        printf("got an error on connection in %s\n", strerror(errno));
   }
}

void Connection::OnInnerConnectionEvent(struct bufferevent* bev, short events){
}

void Connection::OnInnerConnectionRead(struct bufferevent* bev){
}
