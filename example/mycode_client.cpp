#include <iostream>
#include "MprpcChannel.h"
#include "user.pb.h"

int main()
{
    // 建立连接通道
    MprpcChannel channel("127.0.0.1", 8000);

    // 创建 Stub 对象 (代理)
    // 之后调用 stub.Login如同调用本地函数
    // 所有的网络细节全被 channel 屏蔽了
    fixbug::UserServiceRpc_Stub stub(&channel);

    // 设置请求参数
    fixbug::LoginRequest request;
    request.set_name("zhangsan");
    request.set_pwd("123456");

    // 准备响应对象 (用来接结果)
    fixbug::LoginResponse response;

    // 发起 RPC 调用 (同步阻塞等待结果)
    // RpcChannel->CallMethod 集中做所有事情
    stub.Login(nullptr, &request, &response, nullptr); 

    // 检查结果
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success:" << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
    }

    return 0;
}