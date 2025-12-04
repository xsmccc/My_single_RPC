#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

//专用Socket地址结构体
//socket地址-地址族，   端口号      和  IPv4地址结构体
//          AF_INET  网络字节序表示   sin_addr.s_addr （网络字节序表示）
class InetAddress{
public:
    //构造函数：IP默认监听本地所有网卡（0.0.0.0），port必填
    //explicit强制编译器让InetAddress的写法
    //传端口和IP，在主动创建服务器的时候使用，Bind时候用
    explicit InetAddress(uint16_t port,std::string ip="127.0.0.1");

    //构造函数：接受一个已有的sockaddr_in（主要用于accept返回时）
    //传结构体，在被动接受连接时用，Accept时候用
    //const修饰addr表示只读
    explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr){}

    //(const修饰函数本身，保证内容不被更改，只能查看)
    std::string toIp() const;   //用这个格式返回IP字符串
    uint16_t toPort() const;    //返回端口号

    //helper函数：返回内部结构的指针（给bind/accept用）
    const struct sockaddr_in* getSockAddr() const { return &addr_;}//用于获取addr_结构体
    
    void setSockAddr(const struct sockaddr_in& addr) {addr_ = addr;}//用于修改addr_结构体

private:
    struct sockaddr_in addr_;
};