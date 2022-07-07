#include <iostream>
#include <event2/event.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
 #include <unistd.h>
 #include <thread>
 #include <chrono>
using namespace std;

void test01(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }

        event_config *conf = event_config_new();
        //设置支持文件描述符
        event_config_require_features(conf, EV_FEATURE_FDS);
        event_base* base = event_base_new_with_config(conf);
        if(!base){
                std::cout << "event_base_new_with_config failed!"<< std::endl;
                return;
        }

        int sock = open("/var/log/auth.log", O_RDONLY | O_NONBLOCK, 0);
        if(!sock){
                std::cout << "open failed!"<< std::endl;
                return;
        }
        //文件指针移到末尾
        lseek(sock, 0, SEEK_END);
        event *fev = event_new(base, sock, EV_READ | EV_PERSIST, [](evutil_socket_t fd, short event, void *arg){
                        char buf[1024] = {0};
                        int len = read(fd, buf, sizeof(buf) - 1); //留一位放\0
                        if(len){
                                std::cout << buf << std::flush;
                        }else{
                                std::cout << "no data"<< std::endl;
                                std::this_thread::sleep_for(std::chrono::seconds(2));
                        }
                        // std::cout << "..."<< std::flush;
        }, event_self_cbarg());
        event_add(fev, NULL);
        //主循环
        event_base_dispatch(base);
        event_config_free(conf);
        event_base_free(base);
        return;
}

int main(){
        test01();
        return 0;
}