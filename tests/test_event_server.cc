#include <iostream>
#include <event2/event.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
using namespace std;
const static int SPORT = 5001;
void test01(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }

        event_base *base = event_base_new();
        std::cout << "test event server"<< std::endl;
        //创建socket
        evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
        if(!sock){
                std::cout << "socket error:  "  << strerror(errno) << std::endl;
        }
        //设置地址复用和非阻塞
        evutil_make_listen_socket_reuseable_port(sock);
        evutil_make_socket_nonblocking(sock);

        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(SPORT);

        //绑定端口和地址
        int rt = ::bind(sock,(sockaddr*)&sin, sizeof(sin));
        if(rt != 0){
                std::cerr << "bind error:" << strerror(errno) << std::endl;
        }

        //开始监听
        listen(sock, 10);
        //开始接收连接事件,默认水平触发
        event *ev = event_new(base, sock, EV_READ|EV_PERSIST, [](evutil_socket_t s, short w, void *arg){
                        std::cout << "listen_cb"<< std::endl;
                        sockaddr_in sin;
                        socklen_t size = sizeof(sin);
                        evutil_socket_t client  = accept(s, (sockaddr*)&sin, &size);
                        char ip[16] = {0};
                        evutil_inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof(ip) - 1);
                        std::cout << "client ip is : "  << ip << std::endl; 

                        //客户端数据读取事件
                        event *ev = event_new((event_base*)&arg, client, EV_READ|EV_PERSIST, [](evutil_socket_t s, short w, void *arg){
                                //判断超时
                                if(w & EV_TIMEOUT){
                                        std::cout << "timeout" << std::endl;
                                        event_free((event*)&arg);
                                        evutil_closesocket(s);
                                        return;
                                }
                                //需要清理event
                                char buf[1024] = {0};
                                int len = recv(s, buf, sizeof(buf) - 1, 0);
                                if(len > 0){
                                        std::cout << buf << std::endl;
                                        send(s, "ok", 2, 0);
                                }else {
                                        
                                        std::cout << "."<< std::flush;
                                        event_free((event*)&arg);
                                        evutil_closesocket(s);
                                }
                        }, event_self_cbarg());
                        timeval t = {10, 0};
                        event_add(ev, &t);
        }, base);

        event_add(ev, 0);
        event_base_dispatch(base);
        event_base_free(base);
        return;
}

int main(){
        test01();
        return 0;
}