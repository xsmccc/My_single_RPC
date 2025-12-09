#include "EventLoop.h"
#include "Server.h"

int main() {
    EventLoop *loop = new EventLoop();
    Server *server = new Server(loop);
    
    //设置3个线程，一个主Accept+3个子IO
    server->setThreadNum(3);
    server->start();

    loop->loop(); 
    
    delete server;
    delete loop;
    return 0;
}