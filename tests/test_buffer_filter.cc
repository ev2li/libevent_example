#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <signal.h>
#include <memory.h>
 #include <string>
using namespace std;
#define SPORT 5001

bufferevent_filter_result filter_in(evbuffer *s,  evbuffer *d, ev_ssize_t limit,
        bufferevent_flush_mode mode, void *arg){
        std::cout << "filter_in"<< std::endl;
        //读取并清理源数据
        char data[1024] = {0};
        int len = evbuffer_remove(s, data, sizeof(data) - 1);
        //把所有字母转成大写
        for (int i = 0; i < len; i++) {
                data[i] = toupper(data[i]);
        }
        evbuffer_add(d, data, len);
        return BEV_OK;
}

bufferevent_filter_result filter_out(evbuffer *s,  evbuffer *d, ev_ssize_t limit,
        bufferevent_flush_mode mode, void *arg){
        std::cout << "filter_out"<< std::endl;
                //读取并清理源数据
        char data[1024] = {0};
        evbuffer_remove(s, data, sizeof(data) - 1);
        std::string str = "";
        str += "=====================\n";
        str += data;
        str += "=====================\n";
        evbuffer_add(d, str.c_str(), str.size());
        return BEV_OK;
}

void read_cb(bufferevent *bev, void *arg){
        std::cout << "read_cb"<< std::endl;
        //读取数据 
        char data[1024] = {0};
        int len = bufferevent_read(bev, data, sizeof(data) - 1);
        std::cout << data << std::endl;
        //回复用户消息，经过输出过滤
        bufferevent_write(bev, data, len);
}

void write_cb(bufferevent *bev, void *arg){
        std::cout << "wirte_cb"<< std::endl;
}

void event_cb(bufferevent *bev, short events, void *arg){
        std::cout << "event_cb"<< std::endl;
}

void listen_cb(struct evconnlistener* e,  evutil_socket_t s , struct sockaddr* a,  int socklen, void *arg){
        event_base *base = (event_base*)arg;
        std::cout << "listen_cb"<< std::endl;
        //创建bufferevent绑定bufferevent filter
        bufferevent *bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
        //绑定bufferevent filter
        bufferevent *bev_filter = bufferevent_filter_new(bev, 
        filter_in, //输入过滤函数
        filter_out, //输出过滤函数 
        BEV_OPT_CLOSE_ON_FREE, //关闭filter的同时关闭bufferevent
        0,  //清理的回调函数
        0 //传递给回调的函数 
        );
        //设置bufferevent回调
        bufferevent_setcb(bev_filter, read_cb, write_cb, event_cb, NULL); //回调函数的参数 
        bufferevent_enable(bev_filter, EV_READ | EV_WRITE); //开启读写权限
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