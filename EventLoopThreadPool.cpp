#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : baseLoop_(baseLoop)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{
}
//
EventLoopThreadPool::~EventLoopThreadPool(){
    //不用手动delete loops_ 随线程自行销毁
    //不用手动delete threads_ unique_ptr类型自行销毁
}

void EventLoopThreadPool::start(const std::function<void(EventLoop*)>& cb){
    started_ = true;

    //创建指定数量的线程
    for(int i = 0;i < numThreads_;i++)
    {
        //创建线程对象
        auto t = std::make_unique<EventLoopThread>(cb);

        //启动线程并获取loop指针
        //等待startloop初始化完毕
        loops_.push_back(t->startLoop());

        //保存线程对象
        threads.push_back(std::move(t));
    }
    //如果没设置线程数，那么整个池只有主线程一个Loop
    if(numThreads_ == 0 && cb)//这里的cb是什么意思
    {
        cb(baseLoop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop(){
    //默认返回主线程Loop
    EventLoop* loop = baseLoop_;

    //轮询算法（Round-Robin）
    if(!loops_.empty())
    {
        loop = loops_[next_];
        next_ = (next_ + 1) % loops_.size();
    }

    return loop;
}