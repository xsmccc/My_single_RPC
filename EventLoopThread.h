#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>

class EventLoop;

class EventLoopThread{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    //线程函数，在子线程里运行
    void threadFunc();

    EventLoop* loop_;               //临界区资源-多线程共享，存在静态条件
    bool exiting_;                  

    std::thread thread_;            //线程对象-封装OS线程
    std::mutex mutex_;              //互斥锁-保护loop_读写
    std::condition_variable cond_;  //条件变量，用于同步
    ThreadInitCallback callback_;   //线程初始化时的回调（留个拓展口）

};