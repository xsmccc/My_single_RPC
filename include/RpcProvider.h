#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include "Server.h"  
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "Buffer.h"

// 定义智能指针别名
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

class RpcProvider
{
public:
    // 发布 RPC 方法的接口
    void NotifyService(google::protobuf::Service *service);

    // 启动服务
    void Run();

private:
    //Rpc供给类创建则有Loop
    EventLoop eventLoop_; 

    struct ServiceInfo
    {
        google::protobuf::Service *m_service; 
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    //
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 回调函数
    void OnConnection(const TcpConnectionPtr&);
    void OnMessage(const TcpConnectionPtr&, Buffer*); 
    void SendRpcResponse(const TcpConnectionPtr&, google::protobuf::Message*);
};