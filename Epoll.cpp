#include "Epoll.h"
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "Channel.h"
//构造函数实现
Epoll::Epoll() : epfd_(-1),events_(1024){
    epfd_ = epoll_create1(0);
    if(epfd_ == -1)
    {
        perror("epoll create error");
        exit(1);//底层错误直接退出程序
    }
};
//析构函数实现
Epoll::~Epoll(){
    if(epfd_ != -1){
        close(epfd_);
    }
};

// void Epoll::addFd(int fd,uint32_t op){
//     struct epoll_event ev;
//     bzero(&ev,sizeof(ev));
//     ev.data.fd = fd;
//     ev.events = op;

//     //调用系统函数epoll_ctl
//     if(epoll_ctl(epfd_,EPOLL_CTL_ADD,fd,&ev) == -1){
//         perror("epoll add error");
//     }
// }

std::vector<Channel*> Epoll::poll(int timeout){
    std::vector<Channel*> active_channels;
    int nfds = epoll_wait(epfd_,&*events_.begin(),events_.size(),timeout);

    if(nfds == -1){
        perror("epoll wait error");
    }
    else if(nfds > 0){
        //有nfds个事件发生了，取出来放入active_events返回给用户
        for(int i=0;i<nfds;i++)
        {
            Channel *ch = (Channel*)events_[i].data.ptr;

            ch->setRevents(events_[i].events);

            active_channels.push_back(ch);
        }
    }

    return active_channels;
};

void Epoll::updateChannel(Channel *channel){
    int fd = channel->getFd();//从channel中获取fd
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));//bzero函数

    ev.data.ptr = channel;//这个指针原本是存什么的？原本是data.fd存fd的，这个时候的ptr呢？
    ev.events = channel->getEvents();//从channel拿希望监听的事件

    if(!channel->getInEpoll()){//若fd不在epoll树上，则加入
        int ret = epoll_ctl(epfd_,EPOLL_CTL_ADD,fd,&ev);
        if(ret == -1) perror("epoll add error");
        channel->setInEpoll(true);
    }
    else{
        //若已经在树上，则修改
        int ret = epoll_ctl(epfd_,EPOLL_CTL_MOD,fd,&ev);
        if(ret == -1) perror("epoll mod error");
    }
}