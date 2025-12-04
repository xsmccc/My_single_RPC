#pragma once
#include <sys/epoll.h>
#include <vector>
#include "Channel.h"

class Epoll{
public:
    Epoll();
    ~Epoll();

    //op即前面写的功能EPOLLIN/EPOLLOUT
    //fd添加到epoll红黑树上
    // void addFd(int fd,uint32_t op);
    void updateChannel(Channel *channel);
    

    //等待时间发生
    //返回vector，内装有active事件
    //timeout = -1表示永远阻塞等待，直到有事件
    std::vector<Channel*> poll(int timeout = -1);

private:
    int epfd_;//epoll 句柄

    //这个vector用来接epoll_wait返回的事件的
    std::vector<struct epoll_event> events_;
};