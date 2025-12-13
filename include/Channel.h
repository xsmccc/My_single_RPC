#pragma once
#include <sys/epoll.h>
#include <functional>

// 前置声明，告诉编译器有这个类，不用include头文件（防止循环引用）
class Epoll;

class Channel{
public:
    //构造函数：Channel 必须属于某个 Epoll，并负责某个 fd（Epoll返回的事件队列里面不是有一堆fd嘛？）
    Channel(Epoll *ep,int fd);
    ~Channel();

    //开启监听事件
    void enableReading(){
        events_ = kReadEvent| kEtEvent;
        update();
    }

    void disableAll() {
        events_ = kNoneEvent;
        update();}

    bool isNoneEvent() const {return events_ == kNoneEvent;}

    //获取fd
    int getFd() const {return fd_;}

    //获取此时监听的事件-给Epoll用
    uint32_t getEvents() const {return events_;}

    //获取实际发生的事件
    uint32_t getRevents() const {return revents_;}
    void setRevents(uint32_t rev) {revents_ = rev;}

    //是否在Epoll树上
    bool getInEpoll() const {return inEpoll_;}
    void setInEpoll(bool in = true) {inEpoll_ = in;}//默认in为真-默认设置inEpoll_为真

    //回调函数
    using EventCallback = std::function<void()>;

    //设置回调函数  如果发生读事件，就执行 cb
    void setCallback(EventCallback cb) {callback_ = cb;}//根据送入的cb函数执行对应函数；

    //处理事件；Epoll返回后，自动调用此函数 执行callback
    void handleEvent();

private:
    void update();
    Epoll *ep_;             //Epoll类
    int fd_;                //标识符
    uint32_t events_;       //监听的事件
    uint32_t revents_;      //当前发生的事件
    bool inEpoll_;          //标记是否在红黑树上

    static const int kNoneEvent = 0;
    static const int kReadEvent = EPOLLIN | EPOLLPRI;
    static const int kWriteEvent = EPOLLOUT;
    static const int kEtEvent = EPOLLET; //用 ET 模式

    EventCallback callback_;//保存用户注册的回调函数
};