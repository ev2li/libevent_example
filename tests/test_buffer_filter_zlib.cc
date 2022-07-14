#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <signal.h>
#include <memory.h>
 #include "zlib_server.hpp"
#include "zlib_client.hpp"


void test(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }

        std::cout << "test libevent server"<< std::endl;
        //创建libevent上下文
        event_base* base = event_base_new();
        if(base){
                std::cout << "event_base_new success!"<< std::endl;
        }

        void Server(event_base *base);
        Server(base);

        void Client(event_base *base);
        Client(base);
        //事件分发处理
        if(base){
                event_base_dispatch(base);
                event_base_free(base);
        }
}

int main(){
   test();
   return 0;
}