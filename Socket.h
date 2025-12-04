#pragma once
#include "InetAddress.h"

// 封装 socket fd
class Socket {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}//初始化列表的写法

    // 析构函数：在这里关闭 sockfd！
    ~Socket();//类外实现

    int fd() const { return sockfd_; }//const-表明在函数内部绝对不会修改当前对象this的任何成员变量；

    void bindAddress(const InetAddress& localaddr); // 调用 bind
    void listen();                                  // 调用 listen
    int accept(InetAddress* peeraddr);              // 调用 accept

    void setReuseAddr(bool on);                     // 调用 setsockopt

    // 禁用拷贝构造 (防止两个对象管理同一个 fd)
    Socket(const Socket&) = delete;             //禁用拷贝构造
    Socket& operator=(const Socket&) = delete;  //禁用赋值运算符重载

private:
    const int sockfd_;
};