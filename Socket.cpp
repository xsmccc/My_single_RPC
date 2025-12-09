#include "Socket.h"
#include "InetAddress.h"
#include <unistd.h>     // close
#include <sys/socket.h> // socket, bind, listen, accept
#include <string.h>     // memset
#include <stdio.h>      // perror
#include <iostream>

//析构函数的类外实现
Socket::~Socket() {//为移动语义做铺垫，move后原本的sockfd_会变成-1，表示其为空，此时就不执行析构1了；即有效资源则释放，无效资源则啥也不干；
    std::cout << "Debug: Socket destructed,fd=" << sockfd_ << std::endl;
    if (sockfd_ != -1) {
        close(sockfd_);
    }
}

//::是全局作用域解析符，强调调用操作系统的bind函数，而不是其他函数；
void Socket::bindAddress(const InetAddress& localaddr) {
    // 这里的 .getSockAddr() 返回的是 struct sockaddr_in*
    // 系统函数 bind 需要 struct sockaddr*，所以要做一个强转
    // 这是因为所有socket编程接口都用的通用socket地址sockaddr
    int ret = ::bind(sockfd_, (struct sockaddr*)localaddr.getSockAddr(), sizeof(struct sockaddr_in));
    if (ret != 0) {
        perror("bind sockfd error");
    }
}

void Socket::listen() {
    // SOMAXCONN 是系统建议的队列最大长度
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret != 0) {
        perror("listen sockfd error");
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    // 根据optval判断是否开启端口复用
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));//允许端口重用
}

//accept只是从监听队列中取出连接，而不关心连接处于何种状态
int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    
    // 核心：调用系统 accept，这会阻塞直到有新连接进来
    // 注意：这里的 :: 表示调用全局函数（系统的），防止和我们自己的 accept 搞混
    int connfd = ::accept(sockfd_, (struct sockaddr*)&addr, &len);
    
    if (connfd >= 0) {
        // 如果成功，把内核吐出来的对端地址，填到传入的 peeraddr 对象里
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}