#include <iostream>
#include <string>
#include "RpcProvider.h"
#include "user.pb.h"

// 1. 定义业务类 (继承自 Protobuf 生成的 RPC 抽象类)
class UserService : public fixbug::UserServiceRpc 
{
public:
    // 具体的本地业务逻辑
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "==================================" << std::endl;
        std::cout << "Doing local service: Login" << std::endl;
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;  
        std::cout << "==================================" << std::endl;
        return true; 
    }

    // 重写基类的虚函数 (框架调用的入口)
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        //框架已经帮我们反序列化好了 request，直接取数据
        std::string name = request->name();
        std::string pwd = request->pwd();

        //做本地业务
        bool login_result = Login(name, pwd);

        //把结果写入 response
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //执行回调 (告诉框架：我办完了，你可以把 response 发回去了)
        done->Run();
    }
};

int main()
{
    //启动框架
    RpcProvider provider;
    
    // 把 UserService 对象注册到框架里，这样收到 "UserService" 请求时才能找到它
    provider.NotifyService(new UserService());

    //启动网络服务
    provider.Run();

    return 0;
}