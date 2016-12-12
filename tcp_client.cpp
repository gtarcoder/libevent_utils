#include"tcp_client.h"
#include"utils.h"

void TcpClient::KeepAliveCallback(evutil_socket_t fd, short event, void* arg){
    printf("in %s:%s\n", __FILE__, __func__);
    TcpClient* client = (TcpClient*)arg;
    //reconnect
    if(!client->connected_){
        client->DoConnect();
    }
}

TcpClient::TcpClient(std::string peer_ip, uint16_t peer_port, uint16_t local_port, uint16_t index):
Connection(peer_ip, peer_port, local_port, index)
{
}

TcpClient::~TcpClient(){
}

bool TcpClient::Start(){
    
    if(!DoConnect())
        return false;    

    if(SetKeepLive(fd_, 20, 5, 3) < 0){
        printf("set keep alive failed in %s:%s\n", __FILE__, __func__);
        return false;
    }
    if(SetRecvSendBufSize(fd_, 0xfffff) < 0){
        printf("set recv sned bufsize failed in %s:%s\n", __FILE__, __func__);
        return false;
    }

    //create a timer to keep connection alive
    keepalive_event_ = event_new(base_, -1, EV_PERSIST, TcpClient::KeepAliveCallback, this);
    if(!keepalive_event_){
        printf("create event failed in %s:%s!\n", __FILE__, __func__);
        return false;
    }
     
    struct timeval tv = {10, 0};
    evtimer_add(keepalive_event_, &tv);
    return true;
}

void TcpClient::OnPeerRead(){
    //this is for test
    printf("received data from peer, length = %d, data = %s\n", buffer_len_, buffer_);
    usleep(1000000);
    
    char *msg = "hello server!\n";
    Write(msg, strlen(msg));
}

void TcpClient::OnPeerEvent(short events){
   if (events == BEV_EVENT_CONNECTED){
        printf("connected!\n");
        connected_ = true;

        //this is for test
        usleep(1000000);
        char *msg = "hello server!\n";
        Write(msg, strlen(msg));

   }else if (events & BEV_EVENT_EOF){
        evutil_closesocket(fd_);
        fd_ = -1;
        bufferevent_setfd(peer_event_, -1);
        connected_ = false;
        printf("connection closed!\n");
   }else if (events & BEV_EVENT_ERROR){
        printf("got an error on TcpClient: %s\n", strerror(errno));
   }
}

bool TcpClient::DoConnect(){
    if(fd_ < 0){
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if(fd_ < 0){
            printf("create socket failed in %s:%s\n", __FILE__, __func__);
            return false;
        }
        bufferevent_setfd(peer_event_, fd_);
    }

    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(peer_ip_.c_str());
    peer_addr.sin_port = htons(peer_port_);
    int ret = bufferevent_socket_connect(peer_event_, (const sockaddr*)&peer_addr, sizeof(peer_addr));
    if (ret < 0){
        printf("bufferevent socket connect failed in %s:%s!\n", __FILE__, __func__);
        return false;
    }
    return true;
}
