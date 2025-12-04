#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h" 
#include "Channel.h"


using namespace std;

// 【回调函数 1】处理客户端发来的消息
// 参数: channel 指针，为了方便我们在断开连接时，把这个 channel 从 epoll 树上删掉
void handleReadMessage(Channel* channel) {
    int sockfd = channel->getFd();
    char buf[1024] = {0};
    ssize_t n = read(sockfd, buf, sizeof(buf));

    if (n > 0) {
        cout << "Recv from " << sockfd << ": " << buf << endl;
        write(sockfd, buf, n); // Echo 回去
    } else {
        if (n == 0) {
            cout << "Client disconnected: " << sockfd << endl;
        } else {
            perror("read error");
        }
        // 简单粗暴处理：断开就关闭。
        // 注意：在完整版中，这里需要销毁 channel 对象，防止内存泄漏。
        // 但今天为了跑通逻辑，我们先只 close fd。
        close(sockfd); 
        // 实际上 Channel 应该有一个 disableReading() 之类的清理操作，今天先跳过
    }
}

// 【回调函数 2】处理新连接
// 参数: serv_sock 是为了调用 accept; ep 是为了把新连接加入监控
void handleNewConnection(Socket* serv_sock, Epoll* ep) {
    InetAddress client_addr(0);
    int connfd = serv_sock->accept(&client_addr);
    
    if (connfd != -1) {
        cout << "New client connected: " << client_addr.toIp() 
             << ":" << client_addr.toPort() << " fd=" << connfd << endl;

        // ⚡️ 核心步骤：
        // 1. 为这个新客人(connfd) 创建一个专属秘书 (Channel)
        // 注意：这里 new 出来的对象，在简单版里很难管理生命周期，暂时先 new 着不 delete
        // 以后会有 TcpConnection 类来管理它。
        Channel *clientChannel = new Channel(ep, connfd);
        
        // 2. 告诉秘书：如果客人说话了，你就调 handleReadMessage
        clientChannel->setCallback(std::bind(handleReadMessage, clientChannel));
        
        // 3. 开启监听 (enableReading 会自动调用 ep->updateChannel)
        clientChannel->enableReading();
    }
}


int main() {
    // 创建服务器 Socket
    Socket serv_sock(socket(AF_INET, SOCK_STREAM, 0));  //设置IPv4  流服务（TCP）
    InetAddress serv_addr(8000);                        //连接端口8000  默认IP-127.0.0.1
    serv_sock.setReuseAddr(true);                       //开启端口复用
    serv_sock.bindAddress(serv_addr);                   //命名socket，绑定socket地址 InetAddress
    serv_sock.listen();                                 //监听socket    这里的backlog选的最大队列值4096（一般是backlog+1）
    
    //获取IP和port
    cout << "Server is listening on " << serv_addr.toIp() << ":" << serv_addr.toPort() << endl;

    // 创建 Epoll 对象 
    Epoll *ep = new Epoll();

    // 为 server socket 创建Channel
    Channel *servChannel = new Channel(ep,serv_sock.fd());

    // 3. 绑定“新连接来了怎么办”：调用 handleNewConnection
    // std::bind 把参数 serv_sock 和 ep 提前绑死传进去
    servChannel->setCallback(std::bind(handleNewConnection, &serv_sock, ep));

    // 4. 开启监听 (这就相当于把 serv_sock 挂到了 epoll 树上)
    // 监听事件和
    servChannel->enableReading();

    cout << "Server is listening on 8000..." << endl;


    // 4. 事件循环
    while (true) {
        // 返回vector装有活跃事件
        vector<epoll_event> events = ep->poll(); // 默认阻塞

        for(auto &ev : events){
            Channel *channel = (Channel*)ev.data.ptr;\

            channel->setRevents(ev.events);

            channel->handleEvent();
        }
    }

    delete ep;
    delete servChannel;
    return 0;
}