#pragma once
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"

//所以说这个server类就是将前面的都集成起来嘛？
class Server{
public:
    Server(EventLoop *loop);
    ~Server();

    //处理新连接的回调函数
    void handleNewConnection();

private:
    EventLoop *loop_;       //只需要指针，不负责释放
    Socket *acceptor_;      //监听Socket
    InetAddress * addr_;    //地址
    Channel *acceptChannel_;//监听Socket对应的Channel

};