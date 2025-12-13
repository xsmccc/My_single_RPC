#pragma once//只能包含一次
#include <map>
#include <memory>
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include <functional>

class Acceptor;
class Socket;
class TcpConnection;
class Buffer;

class Server{
public:
    Server(EventLoop *loop);
    ~Server();

    void newConnection(Socket *sock);
    void removeConnection(std::shared_ptr<TcpConnection> conn);
    void setThreadNum(int numThreads);

    void setConnectionCallback(const std::function<void(const std::shared_ptr<TcpConnection>&)>& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>& cb) { messageCallback_ = cb; }

    void start();

private:
    EventLoop *loop_;
    std::unique_ptr<Acceptor> acceptor_;// 独占 Acceptor
    bool started_;
    
    // 用 map 存所有的连接，key 是 fd，value 是智能指针
    // 只要这个 map 里还存着，连接就不会断
    std::map<int,std::shared_ptr<TcpConnection>> connections_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    std::function<void(const std::shared_ptr<TcpConnection>&)> connectionCallback_;
    std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)> messageCallback_;
};