#pragma once
#include <functional>
#include <vector>
#include "Epoll.h"
#include <thread>
#include <mutex>

class Channel;//前置声明
class Epoll;

class EventLoop{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //核心接口：启动事件循环
    void loop();
    void quit();

    //在当前线程直接执行，在其他线程，放入队列
    void runInLoop(Functor cb);

    //将任务放入队列
    void queueInLoop(Functor cb);

    //唤醒Loop
    void wakeup();

    //将updateChannel暴露出来给channel调用
    //实际透传给内部ep_
    void updateChannel(Channel* channel);

    Epoll* getEpoll() const {return ep_;}//获取Epoll

    //判断是否在当前线程
    bool isInLoopThread() const {return threadId_ == std::this_thread::get_id();}

private:

    void handleRead();          //处理wakeupFd的读事件
    void doPendingFunctors();   //执行队列里的任务

    Epoll *ep_;//每一个EventLoop都有属于自己的Epoll
    bool quit_;//标志位，控制循环退出

    const std::thread::id threadId_;    //记录创建时的线程ID

    int wakeupFd_;      //eventfd
    std::unique_ptr<Channel> wakeupChannel_;

    std::mutex mutex_;      //保护任务队列
    std::vector<Functor> pendingFunctors_;//任务队列
    bool callingPendingFunctors_;        //标记当前是否正在执行任务
};
