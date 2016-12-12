#include"utils.h"
#include<iostream>
#include<time.h>
#include<ctime>
#include<iomanip>
void InitSocketAddr(std::string& ip, uint16_t port, sockaddr_in* addr){
	memset((char*)addr, 0, sizeof(sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
#ifndef LINUX
	addr->sin_addr.S_un.S_addr = inet_addr(ip.c_str());
#else
	addr->sin_addr.s_addr = inet_addr(ip.c_str());
#endif
}

std::string GetLocalTime(){
	std::time_t now = time(NULL);
	struct tm* t = localtime(&now);
	std::ostringstream os;

	os << std::setw(4) << (t->tm_year + 1900);

	os << '-' << std::setw(2) << std::setfill('0') << t->tm_mon + 1 << '-' << std::setw(2) << std::setfill('0') << t->tm_mday
		<< "  " << std::setw(2) << std::setfill('0') << t->tm_hour
		<< ':' << std::setw(2) << std::setfill('0') << t->tm_min
		<< ':' << std::setw(2) << std::setfill('0') << t->tm_sec << "  ";

	return os.str();
}

int SetNonBlock(int fd){
	int flags;
	flags = fcntl(fd, F_GETFL);
	if (flags < 0) return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0){
    perror("fcntl set nonblock error : ");
    return -1;
  }
  return 0;
}

int SetReuseAddr(int fd){
	int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < -1){
    perror("setsocketopt so_reuseaddr error : ");
    return -1;
  }
  return 0;
}

int SetRecvSendBufSize(int fd, int size){
  if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size,sizeof(size)) < 0){
    perror("setsocketopt so_recbuf error : ");
    return -1;
  }
  if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size,sizeof(size)) < 0){
    perror("setsocketopt so_sndbuf error : ");
    return -1;
  }
  return 0;
}
int SetKeepLive(int fd, int idle, int interval, int count){
  if ( setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void *)&idle, sizeof(idle)) < 0 ){
    perror("setsocketopt tcp_keepidle error : ");
    return -1;
  }
  if ( setsockopt(fd, SOL_TCP,  TCP_KEEPINTVL, (void *)&interval, sizeof(idle)) < 0 ){
    perror("setsocketopt tcp_keepintvl error : ");
    return -1;
  }
  if ( setsockopt(fd, SOL_TCP,  TCP_KEEPCNT, (void *)&count, sizeof(count)) < 0 ){
    perror("setsocketopt tcp_keepcnt error : ");
    return -1;
  }
  return 0;
}


