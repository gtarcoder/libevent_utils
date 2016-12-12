#include"udp_socket.h"
#include"utils.h"
#include<assert.h>

#ifdef ASYNC_UDP
void UdpSocket::ReadCallback(evutil_socket_t fd, short events, void* arg){
    UdpSocket* udp_socket = (UdpSocket*)arg;
    udp_socket->read_len_ = recv(udp_socket->fd_, udp_socket->read_buffer_, 0xffff, 0);
    udp_socket->read_buffer_[udp_socket->read_len_] = '\0';
    assert(udp_socket->read_cb_);
    udp_socket->read_cb_(udp_socket->read_buffer_, udp_socket->read_len_, udp_socket);
}

bool UdpSocket::Bind(const char* local_ip, uint16_t local_port, struct event_base* base){
    base_ = base;
    assert(base_);    
    if(!Bind(local_ip, local_port))
      return false;
    event_ = event_new(base_, fd_, EV_PERSIST|EV_READ, UdpSocket::ReadCallback, this); 
    if(!event_){
        printf("event new failed in %s!\n", __func__);
        return false;
    }
    event_add(event_, NULL);
    return true;
}
#endif


UdpSocket::UdpSocket(){
#ifndef LINUX //in windows system
    static bool was_inited = false;
  	if (wsa_inited_){
		return;
	}

	WSADATA wsaData;
	int iErrorCode;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData)) //µ÷ÓÃWindows Sockets DLL
	{
		printf("Winsock init failed in %s\n", __func__);
		WSACleanup();
		return;
	}
	wsa_inited_ = true;
#endif

#ifdef ASYNC_UDP
    event_ = NULL;
    base_ = NULL;
    read_cb_=NULL;
#endif

    //create a udp socket
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd_ < 0){
        printf("udp create failed in %s\n", __func__);
    }
}
UdpSocket::~UdpSocket(){
#ifdef LINUX
    close(fd_);
#else
    closesocket(fd_);
#endif

#ifdef ASYNC_UDP
    event_free(event_);
#endif
}

bool UdpSocket::Bind(const char* local_ip, uint16_t local_port){
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port);
    local_addr.sin_addr.s_addr = inet_addr(local_ip);
    uint32_t option = 1;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option)) < 0){
       perror("set socket reuse addr\n");
       return false;
    }
    if (bind(fd_, (sockaddr*)&local_addr, sizeof(local_addr)) != 0){
       perror("bind socket\n");
       return false;
    }
    SetRecvSendBufSize(fd_, 0xfffff);
}

void UdpSocket::SetRemoteAddr(const char* remote_ip, uint16_t remote_port){

}

int UdpSocket::Send(char* buf, int len){
	int result = sendto(fd_, buf, len, 0, (sockaddr*)&remote_addr_, sizeof(remote_addr_));
	if (result < 0){
		perror("udp send\n");
	}
    return result;	
}

int UdpSocket::SendTo(char* buf, int len, const char* dst_ip, uint16_t dst_port){
	struct sockaddr_in sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(dst_port);
	sock_addr.sin_addr.s_addr = inet_addr(dst_ip);
	int result = sendto(fd_, buf, len, 0, (sockaddr*)&sock_addr, sizeof(sockaddr));
	if (result < 0){
		perror("udp send\n");
	}
	return result;
}
int UdpSocket::SendTo(char* buf, int len, const struct sockaddr_in& sock_addr){
	int result = sendto(fd_, buf, len, 0, (sockaddr*)&sock_addr, sizeof(sockaddr));
	if (result < 0){
		perror("udp send\n");
	}
	return result;
}

int UdpSocket::Recv(char* buf, int buf_len){
	struct sockaddr_in sock_addr;
#ifdef LINUX
	socklen_t addr_len = sizeof(sock_addr);
#else
	int addr_len = sizeof(sock_addr);
#endif
	int recv_bytes = recvfrom(fd_, buf, buf_len, 0, (sockaddr*)&sock_addr, &addr_len);
	if (recv_bytes < 0){
		perror("udp recv\n");
	}
    return recv_bytes;
}

int UdpSocket::RecvFrom(char* buf, int buf_len, uint32_t* from_ip, uint16_t* from_port){
	struct sockaddr_in sock_addr;
#ifdef LINUX
	socklen_t addr_len = sizeof(sock_addr);
#else
	int addr_len = sizeof(sock_addr);
#endif
	int recv_bytes = recvfrom(fd_, buf, buf_len, 0, (sockaddr*)&sock_addr, &addr_len);
	if (recv_bytes < 0){
		perror("udp recv\n");
	}
	if (from_ip){
		*from_ip = ntohl(sock_addr.sin_addr.s_addr);
	}
	if (from_port){
		*from_port = ntohs(sock_addr.sin_port);
	}

	return recv_bytes;
}
int UdpSocket::WaitMsg(int time_out, int *elapsed_time){
	const int MSG_IDLE = 0, MSG_ARRIVAL = 1;
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(fd_, &read_fds);
	struct timeval time_val;
	time_val.tv_sec = time_out / 1000;
	time_val.tv_usec = time_out % 1000 * 1000;
	int ret = select(fd_ + 1, &read_fds, NULL, NULL, &time_val);
	int result = MSG_IDLE;
	if (ret == -1){
		perror("udp select error\n");
		return result;
	}else if(ret == 0){
		//time out
		*elapsed_time = time_out;
	}else{
		if (FD_ISSET(fd_, &read_fds)){
			*elapsed_time = time_out - (time_val.tv_sec * 1000 + time_val.tv_usec / 1000);
			result = MSG_ARRIVAL;
		}
	}
	return result;
}

