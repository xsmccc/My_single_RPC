#include "Server.h"
#include "Acceptor.h"      //Acceptor
#include "Socket.h"
#include "TcpConnection.h" //TcpConnection
#include <iostream>
#include <functional>
#include <unistd.h>

Server::Server(EventLoop *loop) : loop_(loop) {
    // Server 只负责创建一个 Acceptor 对象。
    acceptor_ = std::make_unique<Acceptor>(loop);//这个（loop）是什么意思？
    
    // 绑定回调
    // 当 Acceptor 收到新连接时，调用 Server::newConnection
    // this 指针就是指“当前的 Server 对象”
    // std::placeholders::_1 是占位符，因为 newConnection 需要一个参数 (Socket*)
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    
    acceptor_->setNewConnectionCallback(cb);
}

Server::~Server() {
    // 智能指针 unique_ptr 会自动释放 acceptor_，不需要 delete
    // map 里的 shared_ptr 也会自动释放
}


void Server::newConnection(Socket *sock) {
    std::cout << "Server::newConnection - new client fd=" << sock->fd() << std::endl;

    // 1. 创建 TcpConnection 对象，用 shared_ptr 管理
    std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(loop_, sock);
    
    // 2. 存入 map (引用计数 +1)
    connections_[sock->fd()] = conn;
    
    // 3. 启动连接
    conn->connectEstablished();
}