#pragma once
#include <memory>
#include <string>
#include <functional>
#include "InetAddress.h"
#include "Buffer.h"

class EventLoop;
class Socket;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>{
public:
    TcpConnection(EventLoop *loop,Socket *sock);
    ~TcpConnection();

    void connectEstablished();

    //定义回调类型
    using Callback = std::function<void(std::shared_ptr<TcpConnection>)>;

    //保存回调接口
    void setCloseCallback(const Callback& cb) {closeCallback_ = cb;}

    int getFd() const;

private:
    Buffer inputBuffer_;  // 接收缓冲区
    Buffer outputBuffer_; // 发送缓冲区 
    EventLoop *loop_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    //成员变量-存回调函数
    Callback closeCallback_;

    void handleRead();//处理读事件回调
    void handleClose(); //处理内部关闭的函数
};