#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

Acceptor::Acceptor(EventLoop *loop) : loop_(loop) {
    acceptSocket_ = std::make_unique<Socket>(socket(AF_INET, SOCK_STREAM, 0));
    std::cout << "DEBUG: Acceptor Created Socket fd=" << acceptSocket_->fd() << std::endl;

    InetAddress addr(8000); // 暂时写死
    acceptSocket_->setReuseAddr(true);
    acceptSocket_->bindAddress(addr);

    acceptChannel_ = std::make_unique<Channel>(loop->getEpoll(), acceptSocket_->fd());

    // 绑定回调：有新连接 -> 执行 acceptConnection
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel_->setCallback(cb);
    std::cout << "DEBUG: Acceptor Constructor Finished" << std::endl; //
}


void Acceptor::listen(){
    acceptSocket_->listen();//开始监听端口
    acceptChannel_->enableReading();//开始关注Epoll事件
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
            // Server 那边会把它接管过去
            Socket *clientSock = new Socket(connfd); 
            newConnectionCallback_(clientSock); 
        } else {
            // 没人管这个连接，那就关掉
            close(connfd);
        }
    }
}