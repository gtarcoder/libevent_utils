#include<iostream>
#include"tcp_server.h"
#include<event2/event.h>
using namespace std;
int main(){
    TcpServer tcp_server;
    struct event_base* base = event_base_new();
    tcp_server.Bind("0.0.0.0", 50000, base);
    event_base_dispatch(base);     
    return 0;
}
