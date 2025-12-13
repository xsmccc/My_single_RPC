# === 1. 编译服务端 (provider) ===
# 最终生成的可执行文件叫 provider
# 它由 main.cpp (你的服务端代码) 和 user.pb.cc (业务协议代码) 组成
add_executable(provider main.cpp user.pb.cc)

# 链接依赖库：
# mprpc: 我们刚才在 src 里生成的框架库
# protobuf: google 的协议库
# pthread: 线程库
target_link_libraries(provider mprpc protobuf pthread)


# === 2. 编译客户端 (consumer) ===
# 最终生成的可执行文件叫 consumer
# 它由 mycode_client.cpp (你的客户端代码) 和 user.pb.cc 组成
add_executable(consumer mycode_client.cpp user.pb.cc)

# 同样链接这三个库
target_link_libraries(consumer mprpc protobuf pthread)