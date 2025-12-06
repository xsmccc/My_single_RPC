#pragma once
#include <functional>
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class Socket;
class InetAddress;
class Channel;

class Acceptor{
public:
    //定义回调类型：当有新连接时，通知server
    using NewConnectionCallback = std::function<void(Socket*)>;

    Acceptor(EventLoop *loop);
    ~Acceptor();

    void acceptConnection();//之前的handleNewConnection
    void setNewConnectionCallback(const NewConnectionCallback &cb) {newConnectionCallback_ = cb;}

private:
    EventLoop *loop_;
    std::unique_ptr<Socket> acceptSocket_;
    std::unique_ptr<Channel> acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
};

