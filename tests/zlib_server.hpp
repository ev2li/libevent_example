#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <signal.h>
#include <memory.h>
 #define SPORT 5001

bufferevent_filter_result filter_in(evbuffer *s, evbuffer *d,  ev_ssize_t limit, 
                bufferevent_flush_mode mode, void *arg){
        //1.接收客户端发送文件名
        char data[1024] = {0};
        int len = evbuffer_remove(s, data, sizeof(data) - 1);
        std::cout <<  "server recv " << data << std::endl;
        evbuffer_add(d, data, len);
        return BEV_OK;
}

void read_cb(bufferevent *bev, void *arg){
        std::cout << "server_read_cb"<< std::endl;
        //2.回复OK
        bufferevent_write(bev, "OK", 2);
}

void event_cb(bufferevent *bev, short events, void *arg){

}

 void listen_cb(struct evconnlistener* e,  evutil_socket_t s , struct sockaddr* a,  int socklen, void *arg){
        std::cout << "listen_cb"<< std::endl;
        event_base *base = (event_base*)arg;
        //1.创建一个bufferevent用来通信
        bufferevent *bev = bufferevent_socket_new(base,  s, BEV_OPT_CLOSE_ON_FREE );
        //2.添加输入过滤，并设置输入回调函数
        bufferevent *bev_filter = bufferevent_filter_new(bev, 
                filter_in,//输入过滤函数;
                0, //输出过滤
                BEV_OPT_CLOSE_ON_FREE, //关闭filter同时关闭bufferevent
                0, //清理回调
                0 //传递参数
        );
        //3.设置回调(读取、事件[用来处理连接断开])
        bufferevent_setcb(bev_filter, read_cb, 0, event_cb, 0 );
        bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}

void Server(event_base *base){
        std::cout << "begin server"<< std::endl;
         //监听端口
        //socket bind listen
        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(SPORT);
        // evconnlistener *ev = 
        evconnlistener_new_bind(base,  //libevent上下文
                                listen_cb, //接收到连接的回调函数 
                                base, //回调函数获取的参数
                                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, //地址重用，listen关闭同时关闭socket
                                10, //连接队列大小，对应listen函数
                                (sockaddr*)&sin,       //绑定的地址和端口
                                sizeof(sin)
                                 );
}