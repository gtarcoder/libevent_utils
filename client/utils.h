#ifndef UTILS_H_
#define UTILS_H_
#include<string>
#include<sstream>

#ifdef LINUX
#include<assert.h>
#include<netinet/in.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
using  SockInt  = int;

#else
#include <stdint.h>
#include <stdio.h>
#include <Winsock2.h> 
#pragma comment(lib, "ws2_32.lib")

using SockInt = SOCKET;
#endif

template<typename T>
std::string Type2String(T& t){
  std::ostringstream os;
  os << t;
  return os.str();
}

template<typename T>
T String2Type(std::string& str){
  std::istringstream ss(str);
  T result;
  ss >> result;
  return result;
}

std::string GetLocalTime();

//for socket use
void InitSocketAddr(std::string& ip, uint16_t port, sockaddr_in* addr);
int SetNonBlock(int fd);
int SetReuseAddr(int fd);
int SetRecvSendBufSize(int fd, int size);
int SetKeepLive(int fd, int idle, int interval, int count);

#endif
