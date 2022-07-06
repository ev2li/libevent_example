#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <signal.h>
#include <memory.h>
 
using namespace std;
#define SPORT 5001
void listen_cb(struct evconnlistener* e,  evutil_socket_t s , struct sockaddr* a,  int socklen, void *arg){
        std::cout << "listen_cb"<< std::endl;
}

void test(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }

        std::cout << "test libevent server"<< std::endl;
        //创建libevent上下文
        event_base* base = event_base_new();

        //监听端口
        //socket bind listen
        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(SPORT);
        evconnlistener *ev =  evconnlistener_new_bind(base,  //libevent上下文
                                listen_cb, //接收到连接的回调函数 
                                base, //回调函数获取的参数
                                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, //地址重用，listen关闭同时关闭socket
                                10, //连接队列大小，对应listen函数
                                (sockaddr*)&sin,       //绑定的地址和端口
                                sizeof(sin)
                                 );

        //事件分发处理
        if(base){
                event_base_dispatch(base);
        }
        
        if(ev){
                evconnlistener_free(ev);
        }

        if(base){
                std::cout << "event_base_new success!"<< std::endl;
        }
        event_base_free(base);
}

int main(){
   test();
   return 0;
}