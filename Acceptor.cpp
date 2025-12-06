#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

Acceptor::Acceptor(EventLoop *loop) : loop_(loop) {
    acceptSocket_ = new Socket(socket(AF_INET, SOCK_STREAM, 0));
    InetAddress *addr = new InetAddress(8000); // 暂时写死
    acceptSocket_->setReuseAddr(true);
    acceptSocket_->bindAddress(*addr);
    acceptSocket_->listen();
    // addr 用完就可以删了，因为 socket 已经绑定进内核了
    delete addr;

    acceptChannel_ = new Channel(loop->getEpoll(), acceptSocket_->fd());
    // 绑定回调：有新连接 -> 执行 acceptConnection
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel_->setCallback(cb);
    acceptChannel_->enableReading();
}

Acceptor::~Acceptor() {

}

void Acceptor::acceptConnection() {
    InetAddress clientAddr(0);
    // 调用 Socket 的 accept
    int connfd = acceptSocket_->accept(&clientAddr);//绑定服务端的Socket地址
    if(connfd != -1){
        if(newConnectionCallback_){
            // 新生成的 socket 包装一下，扔给 Server
            // 我们这里先简单传一个 new 出来的 Socket 对象
            // Server 那边会把它接管过去
            Socket *clientSock = new Socket(connfd); 
            newConnectionCallback_(clientSock); 
        } else {
            // 没人管这个连接，那就关掉
            close(connfd);
        }
    }
}