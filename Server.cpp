#include "Server.h"
#include "Acceptor.h"      //Acceptor
#include "Socket.h"
#include "TcpConnection.h" //TcpConnection
#include <iostream>
#include <functional>
#include <unistd.h>
#include "EventLoopThreadPool.h"

Server::Server(EventLoop *loop) 
    : loop_(loop) 
    , acceptor_(std::make_unique<Acceptor>(loop))//这个（loop）是什么意思？// Server 只负责创建一个 Acceptor 对象。
    , threadPool_(std::make_unique<EventLoopThreadPool>(loop))
    , started_(false) // 1. 初始化标志位
{ 
    // 绑定回调
    // 当 Acceptor 收到新连接时，调用 Server::newConnection
    // this 指针就是指“当前的 Server 对象”
    // std::placeholders::_1 是占位符，因为 newConnection 需要一个参数 (Socket*)
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    
    acceptor_->setNewConnectionCallback(cb);
}

void Server::setThreadNum(int numThreads){
    threadPool_->setThreadNum(numThreads);
}

Server::~Server() {
    // 智能指针 unique_ptr 会自动释放 acceptor_，不需要 delete
    // map 里的 shared_ptr 也会自动释放
}

void Server::start(){
    if(started_) return;
    started_ = true;

    //1. 启动线程池
    threadPool_->start();

    //开启端口监听
    acceptor_->listen();

}


void Server::newConnection(Socket *sock) {
    std::cout << "Server::newConnection - new client fd=" << sock->fd() << std::endl;

    // 1. 创建 TcpConnection 对象，用 shared_ptr 管理
    // std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(loop_, sock);

    //从线程池中取一个ioLoop
    EventLoop *ioLoop = threadPool_->getNextLoop();

    ioLoop->testRisk();

    std::cout<<"New conn on loop: "<<ioLoop << std::endl;

    //把新连接交给ioLoop管理
    std::shared_ptr<TcpConnection> conn = std::make_shared<TcpConnection>(ioLoop,sock);

    connections_[sock->fd()] = conn;
    conn->setCloseCallback(std::bind(&Server::removeConnection,this,std::placeholders::_1));

    //由于connectEstablished要调用channel->enableReading 而channel属于ioLoop（子线程）
    //因此直接在主线程调用ioLoop的函数会有线程安全问题
    //目前先直接调用，后续修改
    conn->connectEstablished();
}

void Server::removeConnection(std::shared_ptr<TcpConnection> conn){
    std::cout << "Debug: Server::removeConnection start,ref_count="<<conn.use_count()<<std::endl;

    int n = connections_.erase(conn->getFd());

    std::cout << "Debug: Server::removeConnection end,ref_count="<<conn.use_count()<<std::endl;
}