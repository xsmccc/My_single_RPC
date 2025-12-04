#include "Server.h"
#include <functional>
#include <string.h>
#include <unistd.h>
#include <iostream>

// 为了简单演示，handleReadMessage 暂时还放在这里作为全局函数
// 实际上以后它会移入 TcpConnection 类
void handleReadMessage(Channel* channel)
{
    int sockfd = channel->getFd();
    char buf[1024] = {0};
    ssize_t n = read(sockfd,buf,sizeof(buf));
    if(n>0){
        std::cout <<"Recv: "<<buf<<std::endl;
        write(sockfd,buf,n);
    }
    else{
        close(sockfd);
        // 注意：这里没有 delete channel，存在内存泄漏，后面几天会修
    }
}

Server::Server(EventLoop *loop) : loop_(loop){
    //初始化Socket和Address
    addr_ = new InetAddress(8000);
    acceptor_ = new Socket(socket(AF_INET, SOCK_STREAM,0));
    acceptor_->setReuseAddr(true);
    acceptor_->bindAddress(*addr_);
    acceptor_->listen();

    //初始化channel
    // 注意：这里用 loop->updateChannel，而不是直接调 epoll
    acceptChannel_ = new Channel(loop->getEpoll(),acceptor_->fd());

    //绑定回调
    std::function<void()> cb = std::bind(&Server::handleNewConnection,this);//还是没能理解，这个this是指哪个类呢？server？

    //开启监听
    acceptChannel_->enableReading();
}

Server::~Server(){
    delete acceptor_;
    delete addr_;
    delete acceptChannel_;
}

void Server::handleNewConnection(){
    InetAddress client_addr(0);
    int connfd = acceptor_->accept(&client_addr);
    std::cout<<"New Connection: "<<connfd<<std::endl;

    //为新连接创建channel
    Channel *clientChannel = new Channel(loop_->getEpoll(),connfd);
    clientChannel->setCallback(std::bind(handleReadMessage,clientChannel));
    clientChannel->enableReading();
}