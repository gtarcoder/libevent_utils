#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_
#include<stdint.h>
#include<functional>
#ifdef ASYNC_UDP
    #include<event2/event.h>
#endif
class UdpSocket{
public:
    UdpSocket();
    virtual ~UdpSocket();
#ifdef ASYNC_UDP
    bool Bind(const char* local_ip, uint16_t local_port, struct event_base*);
#endif
    bool Bind(const char* local_ip, uint16_t local_port);

    void SetRemoteAddr(const char* remote_ip, uint16_t remote_port);
    int SendTo(char* buf, int len, const char* dst_ip, uint16_t dst_port); 
    int SendTo(char* buf, int len, const struct sockaddr_in& target_addr); 
    int Send(char* buf, int len);
    int RecvFrom(char* buf, int buf_len, uint32_t* from_ip, uint16_t* from_port);
    int Recv(char* buf, int buf_len);
    int WaitMsg(int time_out, int* elapsed_time);

    inline void SetDataReadCallback(std::function<void(char*, int)> cb){read_cb_ = cb;};
private:
#ifdef ASYNC_UDP
    static void ReadCallback(evutil_socket_t fd, short events, void* arg);
    std::function<void(char*, int len)> read_cb_;
    struct event_base* base_;
    struct event*      event_;
    char read_buffer_[0xffff];
    int read_len_;
#endif
    int                fd_;
    struct sockaddr    remote_addr_;
};
#endif
