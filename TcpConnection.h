#pragma once
#include <memory>
#include <string>
#include <functional>
#include "InetAddress.h"

class EventLoop;
class Socket;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>{
public:
    TcpConnection(EventLoop *loop,Socket *sock);
    ~TcpConnection();

    void connectEstablished();

    int getFd() const;

private:
    EventLoop *loop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    void handleRead();//处理读事件回调
};