#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>
#include <iostream>

TcpConnection::TcpConnection(EventLoop *loop, Socket *sock) 
    : loop_(loop), socket_(sock) // socket_ 此时接管了 sock 的所有权 (unique_ptr)
{
    // 为这个连接创建专属 Channel
    channel_ = std::make_unique<Channel>(loop_->getEpoll(), socket_->fd());//接管client的Socket套接字；
    
    // 绑定回调：读事件 -> handleRead
    //这里不能在构造函数里调用 shared_from_this()！
    // 因为对象还没构造完，引用计数还是 0。必须在 connectEstablished 里做。
    std::function<void()> cb = std::bind(&TcpConnection::handleRead, this);
    channel_->setCallback(cb);
}

TcpConnection::~TcpConnection() {
    std::cout << "TcpConnection::~TcpConnection fd=" << socket_->fd() << std::endl;
}

// 连接建立完成（Server 创建完对象后调用）
void TcpConnection::connectEstablished() {
    channel_->enableReading(); // 开启 Epoll 监听
}

int TcpConnection::getFd() const {
    return socket_->fd();
}

void TcpConnection::handleRead() {
    int sockfd = channel_->getFd();
    char buf[1024] = {0};
    ssize_t n = read(sockfd, buf, sizeof(buf));
    
    if (n > 0) {
        std::cout << "Recv: " << buf << std::endl;
        write(sockfd, buf, n); // Echo
    } else if (n == 0) {
        std::cout << "Client disconnected fd=" << sockfd << std::endl;
        // 这里非常关键：
        // 暂时只做简单处理：删除 channel，关闭连接
        // delete this; //绝对禁止在 shared_ptr 管理的对象里 delete this
    }
}