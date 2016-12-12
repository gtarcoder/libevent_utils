#ifndef COMMON_DEFS_H_
#define COMMON_DEFS_H_
//self-define structure, is used as the parameter passed into libevent callback
class TcpServer;
class Connection;
struct ConnEventArg{
    TcpServer* server;
    Connection* conn;
};
#endif
