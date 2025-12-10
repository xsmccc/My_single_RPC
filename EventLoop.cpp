#include "EventLoop.h"
#include "Channel.h"
#include <vector>
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>

//创建eventfd（非阻塞+exec时关闭）
int creatEventfd(){
    int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);//非阻塞且执行
    if(evtfd < 0){
        perror("eventfd error");
        exit(1);
    }
    return evtfd;
}


//构造函数中
EventLoop::EventLoop() 
    : ep_(new Epoll())
    , quit_(false)
    , threadId_(std::this_thread::get_id())
    , wakeupFd_(creatEventfd())
    , wakeupChannel_(new Channel(ep_,wakeupFd_))
    , callingPendingFunctors_(false)
{
    //配置wakeupChannel，监听读事件
    //当有人notify时，Epoll会触发handleRead
    wakeupChannel_->setCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    wakeupChannel_->disableAll();//channel中没有disableAll
    ::close(wakeupFd_);
    delete ep_;
}

//唤醒后回调-读取eventfd中的8个字节，清空buffer
//如若不读，wait会立刻返回（LF）
void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_,&one, sizeof(one));
    if(n != sizeof(one)){
        perror("EventLoop::handlRead reads warning");
    }
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)){
        perror("EventLoop::wakeup writes warning");
    }
}

void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread()){
        cb();
    }
    else{
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    //如果不是当前线程，必须唤醒
    //如果是当前线程，但在执行任务的队列中，则在回调中添加了新任务，亦唤醒，防止漏掉
    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup();
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    // 交换 (swap)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const auto& functor : functors) {
        functor();
    }
    
    callingPendingFunctors_ = false;
}


void EventLoop::loop(){
    while(!quit_){
        //调用epoll_wait等待事件
        std::vector<Channel*> chs = ep_->poll();
        //遍历处理事件
        for(auto it = chs.begin();it != chs.end();++it)
        {
            (*it)->handleEvent();//自行处理
        }

        doPendingFunctors();
    }
}

void EventLoop::quit() { quit_ = true; }


void EventLoop::updateChannel(Channel *channel){
    ep_->updateChannel(channel);
}