#include <iostream>
#include <string>
#include "rpcheader.pb.h"

using namespace std;

// 网络库回调的数据 (Buffer)
void OnMessage(string recv_buf) {

    //读取前 4 个字节 (header_size)
    uint32_t header_size = 0;
    // copy 也是从 string 里的 char* 拷贝到 内存地址
    recv_buf.copy((char*)&header_size, 4, 0);

    //根据 header_size 读取 RpcHeader 的原始字符流
    // substr(起始下标, 长度)
    string rpc_header_str = recv_buf.substr(4, header_size);

    //反序列化 RpcHeader
    mprpc::RpcHeader rpcHeader;
    string service_name;
    string method_name;
    uint32_t args_size;

    if (rpcHeader.ParseFromString(rpc_header_str)) {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    } else {
        cout << "rpc_header_str:" << rpc_header_str << " parse error!" << endl;
        return;
    }

    //获取真正的参数字符流
    // 起始位置 = 4字节长度头 + header本身长度
    string args_str = recv_buf.substr(4 + header_size, args_size);

    // ===============================================
    // 打印调试信息
    // ===============================================
    cout << "==================================" << endl;
    cout << "header_size: " << header_size << endl; 
    cout << "service_name: " << service_name << endl;
    cout << "method_name: " << method_name << endl;
    cout << "args_size: " << args_size << endl;
    cout << "args_str: " << args_str << endl;
    cout << "==================================" << endl;
}

int main() {
    // 模拟发送端发送的数据
    // 构造一个 binary data: [header_size] + [RpcHeader] + [args]
    
    //构造头
    mprpc::RpcHeader header;
    header.set_service_name("UserService");
    header.set_method_name("Login");
    header.set_args_size(6); // 假设参数长度是 6 ("123456")

    string header_str;
    header.SerializeToString(&header_str);//序列化

    //组装
    uint32_t header_size = header_str.size();
    string send_str;
    send_str.insert(0, string((char*)&header_size, 4)); // 写入4字节长度
    send_str += header_str;
    send_str += "123456"; // 模拟的 args 参数

    //模拟网络接收
    //调用处理函数
    OnMessage(send_str);

    return 0;
}