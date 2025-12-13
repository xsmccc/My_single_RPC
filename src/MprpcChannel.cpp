#include "MprpcChannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

/*
Header的格式：
header_size + service_name method_name args_size + args
*/
// 所有通过 stub 代理对象调用的 rpc 方法，都走到这里了
// 统一做 Rpc 方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                                google::protobuf::RpcController* controller, 
                                const google::protobuf::Message* request,
                                google::protobuf::Message* response,
                                google::protobuf::Closure* done)
{
    // 获取服务名和方法名
    //通过method描述符，反向查出服务名和方法名
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();    // "UserServiceRpc"
    std::string method_name = method->name(); // "Login"

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        std::cout << "serialize request error!" << std::endl;
        return;
    }

    // 定义 rpc 的请求 header，并填入所有元数据
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    //序列化Header
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        std::cout << "serialize rpc header error!" << std::endl;
        return;
    }

    // 组织待发送的 rpc 请求的字符串
    std::string send_rpc_str;
    // [4字节头长度]
    send_rpc_str.insert(0, std::string((char*)&header_size, 4)); 
    // [头数据]
    send_rpc_str += rpc_header_str; 
    // [参数数据]
    send_rpc_str += args_str; 

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl; 
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
    std::cout << "service_name: " << service_name << std::endl; 
    std::cout << "method_name: " << method_name << std::endl; 
    std::cout << "args_size: " << args_size << std::endl; 
    std::cout << "============================================" << std::endl;

    // 使用 TCP 编程，发送 rpc 请求
    // 注意：这里用的是最原始的 socket，没有用 muduo，因为 client 端通常是阻塞等待结果的
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        // 简单处理错误，实际项目可能需要 setController Error
        perror("create socket error! \n");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_port);
    server_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());

    // 连接服务器
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        close(clientfd);
        perror("connect error! \n");
        return;
    }

    // 发送数据
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        perror("send error! \n");
        return;
    }

    // 接收 rpc 请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    // 阻塞接收
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        close(clientfd);
        perror("recv error! \n");
        return;
    }

    // 7. 反序列化响应数据到 response
    // std::string(recv_buf, 0, recv_size) 
    // bug修复：recv_buf 是数组，直接 parse 即可，不需要转 string 避免多余拷贝
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);
        std::cout << "parse error! response_str:" << recv_buf << std::endl;
        return;
    }

    close(clientfd);
}