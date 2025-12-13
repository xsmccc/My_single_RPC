#pragma once
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <string>

class MprpcChannel : public google::protobuf::RpcChannel
{
public:
    // 构造函数接收 IP 和 Port
    MprpcChannel(std::string ip, uint16_t port) : m_ip(ip), m_port(port) {}

    //所有通过 stub 调用的 rpc 方法，最终都会统一调用到这个 CallMethod

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, 
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done);

private:
    std::string m_ip;
    uint16_t m_port;
};