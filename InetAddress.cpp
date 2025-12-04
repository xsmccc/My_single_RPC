#include "InetAddress.h"
#include <string.h> // 为了使用 bzero 或 memset

// 1. 实现构造函数
// 这里的 InetAddress:: 意思是：我现在要写 InetAddress 类里的那个构造函数
InetAddress::InetAddress(uint16_t port, std::string ip) {
    // 这三行是你昨天的代码，我只是搬过来了
    bzero(&addr_, sizeof(addr_));             // 清空内存
    addr_.sin_family = AF_INET;               // IPv4
    addr_.sin_port = htons(port);             // 端口转网络字节序（小端变大端，8000-0x401F（16415））
    
    // IP地址转换
    // c_str() 把 string 转成 char*，因为 inet_addr 只认 char*
    //inet_addr将IP转成整数类型，存入addr_结构体；
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); 
}

// 2. 实现 toIp 方法
//用于后续日志打印
std::string InetAddress::toIp() const {
    // inet_ntoa 负责把网络字节序的整数转成字符串 "192.168.x.x"
    char* buf = inet_ntoa(addr_.sin_addr);
    return buf; 
}

// 3. 实现 toPort 方法
uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port); //从大端转变为小端-network to host
}