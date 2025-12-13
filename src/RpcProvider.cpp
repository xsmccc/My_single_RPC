#include "RpcProvider.h"
#include "rpcheader.pb.h"
#include <functional>
#include <iostream>
#include "Logger.h"

// 1. 注册服务 
void RpcProvider::NotifyService(google::protobuf::Service *service)
{

    ServiceInfo service_info;
    
    //获取Descriptor
    //GetDescriptor()由protobuf生成，可拿到服务的所有元数据
    const google::protobuf::ServiceDescriptor *psetviceDesc = service->GetDescriptor();
    
    //获取服务的名字-什么服务
    std::string service_name = psetviceDesc->name();
    
    //拿到method的数量-什么方法
    int methodCnt = psetviceDesc->method_count();

    //遍历每一个方法
    for (int i = 0; i < methodCnt; ++i)
    {
        //获取第i个方法的描述符
        const google::protobuf::MethodDescriptor* pmethodDesc = psetviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        
        //将方法名和方法描述符存入unordered_map
        service_info.m_methodMap.insert({method_name, pmethodDesc});
    }

    //记录服务对象本身
    service_info.m_service = service;
    
    //服务名-》服务详细信息 对应
    m_serviceMap.insert({service_name, service_info});
}

// 2. 启动服务
void RpcProvider::Run()
{
    // 尽管 Server 构造不需要 IP/Port，但通常我们打印一下日志
    std::string ip = "127.0.0.1";
    uint16_t port = 8000;
    
    // Server 构造函数只传 loop (严格按照你的 Server.h)
    Server server(&eventLoop_);

    // 绑定连接回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));

    // 绑定消息回调 
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2));

    // 设置线程数量
    server.setThreadNum(4);

    LOG_INFO("RpcProvider start service at ip:%s port:%d", ip.c_str(), port);

    // 启动服务
    // (注意：如果你的 Server 构造函数不传 Port，那你的 Acceptor 内部可能是写死端口的，或者是你在 Server 里有其他 bind 方法)
    // (按照目前代码，我们先保证能编译通过并运行)
    server.start();
    eventLoop_.loop();
}

// 3. 连接回调
void RpcProvider::OnConnection(const TcpConnectionPtr &conn)
{
    // 保持为空，不做任何处理
}

// 4. 读写回调 (Day 10 的核心逻辑)
void RpcProvider::OnMessage(const TcpConnectionPtr &conn, Buffer *buffer)
{
    // 防止空包或者不够读出头长度
    // 如果缓冲区数据少于4个字节，说明连头长度都读不出来，直接返回等下次
    if (buffer->readableBytes() < 4) return;

    // 偷看（Peek）前4个字节，获取 header_size  计算后面的数据长度
    // 注意：这里用 peek()，绝对不能用 retrieve()，因为如果数据不够，我们还得把数据留在缓冲区里
    const char* data = buffer->peek();
    uint32_t header_size = 0;
    // 直接按内存读取前4个字节（注意字节序，这里假设本机字节序一致）    二进制数转整数
    header_size = *reinterpret_cast<const uint32_t*>(data);
    
    // 只有当 缓冲区里的数据 >= (4字节长度 + header长度) 时，才说明我们收到了至少一个完整的包
    // 否则说明数据还没发完（TCP拆包了），直接 return，等数据收齐了再来处理
    // 简化了逻辑，应该还要加上args_size
    if (buffer->readableBytes() < 4 + header_size) {
        return; 
    }

    // ============================================
    // 若满足上述条件，则取出数据
    // ============================================
    
    // 从缓冲区获取数据 
    std::string recv_buf = buffer->retrieveAllAsString();

    // 读取 Header
    // 此时 recv_buf 的大小至少是 4 + header_size
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    
    //反序列化Header
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // 读取 Args
    // 之前并未检查 (4 + header_size + args_size)，保证Body的完整性
    // 因为 ParseFromString 遇到数据截断通常只会解析失败，不会像 substr 那样直接崩溃
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    LOG_INFO("==================================");
    LOG_INFO("header_size: %d", header_size); 
    LOG_INFO("service_name: %s", service_name.c_str()); 
    LOG_INFO("method_name: %s", method_name.c_str()); 
    LOG_INFO("args_size: %d", args_size);
    LOG_INFO("==================================");

    // 查表分发
    //查表看是否有service_name的服务
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << " is not exist!" << std::endl;
        return;
    }

    //查表看是否有method_name的方法
    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }

    //取具体服务对象和方法描述符
    google::protobuf::Service *service = it->second.m_service; 
    const google::protobuf::MethodDescriptor *method = mit->second;

    //最难的地方-反射
    // 动态创建请求和响应对象！！！
    //根据method创建出LoginRequest对象
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error!" << std::endl;
        return;
    }
    //创建出空的响应对象
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 绑定 Closure 回调，等业务做完了调用SendRpcResponse发结果
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const TcpConnectionPtr&, 
                                                                    google::protobuf::Message*>
                                                                    (this, 
                                                                     &RpcProvider::SendRpcResponse, 
                                                                     conn, response);

    // 执行业务
    service->CallMethod(method, nullptr, request, response, done);
}

// 5. 发送响应
void RpcProvider::SendRpcResponse(const TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) 
    {
        // TcpConnection::send 已经在上面改好了，这里直接调用
        conn->send(response_str); 
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl;
    }
}