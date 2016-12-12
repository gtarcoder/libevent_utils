#include<iostream>
#include"tcp_client.h"
#include<event2/event.h>
using namespace std;
int main(){
    struct event_base* base = event_base_new();
    TcpClient client(string("127.0.0.1"), 50000);
    client.Initialize(base, -1);
    client.Start();
    event_base_dispatch(base);
    return 0;
}
