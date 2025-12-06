#include "Channel.h"
#include "Epoll.h"

//构造函数的类外实现
Channel::Channel(Epoll *ep,int fd)//为什么直接挂在Epoll，应该是Eventloop吧
    :ep_(ep), fd_(fd), events_(0), revents_(0),inEpoll_(false)//类初始化
{
}

//析构函数的类外实现
Channel::~Channel(){
    //删除epoll中的fd
}

void Channel::enableReading(){
    events_ = EPOLLIN | EPOLLET;
    ep_->updateChannel(this);
}

void Channel::handleEvent(){
    //读事件
    if(revents_ & (EPOLLIN | EPOLLPRI)){
        if(callback_){
            callback_();//执行绑定的回调函数
        }
    }
}