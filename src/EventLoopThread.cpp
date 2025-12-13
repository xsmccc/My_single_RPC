#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                const std::string& name)
                :loop_(nullptr)
                ,exiting_(false)
                ,callback_(cb)
{
}

EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    if(loop_ != nullptr){
        if(thread_.joinable()){
            thread_.join();
        }
    }
}

//主线程函数
EventLoop* EventLoopThread::startLoop(){
    //启用新线程，执行threadFunc
    thread_ = std::thread(std::bind(&EventLoopThread::threadFunc,this));

    //等待子线程把Loop创建完
    //利用锁和条件变量保证Loop创建完成
    {
        //获取互斥锁，保护共享变量loop_
        std::unique_lock<std::mutex> lock(mutex_);
        //检查条件loop_是否赋值
        while(loop_ == nullptr){

            //wait函数执行unlock解锁，子线程有机会拿到锁；挂起线程，省CPU；notify唤醒后，重新lock
            cond_.wait(lock);
        }
    }//lock自动析构

    //返回loop指针给Server
    return loop_;
}

//子线程函数
void EventLoopThread::threadFunc(){
    //子线程栈上创建一个EventLoop
    //即one Loop Per Thread核心——子线程Loop
    EventLoop loop;

    //初始化回调
    if(callback_){
        callback_(&loop);
    }

    //Loop指针暴露给主线程，并通知它
    {
        std::unique_lock<std::mutex> lock(mutex_);//自动上锁
        loop_ = &loop;
        cond_.notify_one();//唤醒主线程
    }

    loop.loop();

    //循环结束，清理指针
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
