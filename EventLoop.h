#pragma once
#include <functional>
#include <vector>
#include "Epoll.h"

class Channel;//前置声明
class Epoll;

class EventLoop{
public:
    EventLoop();
    ~EventLoop();

    //核心接口：启动事件循环
    void loop();

    //测试函数
    void testRisk(){
        // 模拟复杂操作：往 vector 里塞数据
        for(int i=0; i<10000; ++i) {
            vec_.push_back(i);
        }
    }

    //将updateChannel暴露出来给channel调用
    //实际透传给内部ep_
    void updateChannel(Channel* channel);

    Epoll* getEpoll() const {return ep_;}//获取Epoll

private:
    Epoll *ep_;//每一个EventLoop都有属于自己的Epoll
    bool quit_;//标志位，控制循环退出

    //破坏性实验
    std::vector<int> vec_;
};
