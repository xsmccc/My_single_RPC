#pragma once
#include <vector>
#include <string>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool{
public:
    EventLoopThreadPool(EventLoop* baseLoop);   //主线程的Loop
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) {numThreads_ = numThreads;}

    void start(const std::function<void(EventLoop*)>& cb = std::function<void(EventLoop*)>());//这个定义没看懂，是一个const std::function<void(EventLoop*)>类型的cb为入口参数嘛？

    EventLoop* getNextLoop();
private:
    EventLoop* baseLoop_;   //主线程Loop（若线程为0，则用这个）
    bool started_;
    int numThreads_;
    int next_;          //轮询下标

    //保存所有创建的线程对象（负责析构）
    std::vector<std::unique_ptr<EventLoopThread>> threads;

    //保存所有子线程运行的Loop指针（快速获取但不负责析构）
    std::vector<EventLoop*> loops_;
};