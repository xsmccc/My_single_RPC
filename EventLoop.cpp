#include "EventLoop.h"
#include "Channel.h"
#include <vector>

//构造函数中只创建Epoll对象
EventLoop::EventLoop() : ep_(new Epoll()),quit_(false){
        
}

EventLoop::~EventLoop(){
    delete ep_;
}

void EventLoop::loop(){
    while(!quit_){
        //调用epoll_wait等待事件
        std::vector<Channel*> chs;

        testRisk();
        
        //修改poll的返回值
        chs = ep_->poll();

        //遍历处理事件
        for(auto it = chs.begin();it != chs.end();++it)
        {
            (*it)->handleEvent();//自行处理
        }
    }
}

//updateChannel函数不是epoll和channel的连接嘛，怎么再EventLoop中也有
void EventLoop::updateChannel(Channel *channel){
    ep_->updateChannel(channel);
}