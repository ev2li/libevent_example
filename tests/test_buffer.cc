#include <iostream>
#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/bufferevent.h>
#include <signal.h>
#include <event2/listener.h>
#include <string.h>
#include <string>

static std::string recvstr = "";
static int recvCount = 0;

void read_cb(bufferevent *be, void *arg){
        std::cout << "[R]" << std::endl;
        char data[1024] = {0};
        //读取输入缓冲数据
        int len = bufferevent_read(be, data, sizeof(data) - 1);
        std::cout << data << std::flush;
        if(len <= 0){
                return;
        }
        recvstr += data;
        recvCount += len;
        //std::cout <<  "[" << data  << std::endl;
        //发送数据(写入到输出缓冲) 
       /*if(strstr(data, "quit") != NULL){
                std::cout << "quit"<< std::endl;
                bufferevent_free(be);
        }*/
        bufferevent_write(be, "ok", 3);
}

void write_cb(bufferevent *be, void *arg){
        std::cout << "[W]" << std::endl;
}

//错误，超时（连接断开会进入）
void event_cb(bufferevent *be, short events, void *arg){
         std::cout << "[E]" << std::endl;
         //读取超时事件发生后，数据 读取停止
         if(events & BEV_EVENT_TIMEOUT && events & BEV_EVENT_READING){
                //读取缓冲区
                char data[1024] = {0};
                int len = bufferevent_read(be, data, sizeof(data) - 1);
                if(len > 0 ){
                        recvCount += len;
                        recvstr += data;
                }
                std::cout << "BEV_EVENT_TIMEOUT"<< std::endl;
                bufferevent_free(be); //断开连接
                std::cout << recvstr << std::endl;
                std::cout << "recvCoount"  << recvCount<< std::endl;
                return;
         }else if(events & BEV_EVENT_ERROR){
                bufferevent_free(be); //断开连接
                return;
         }else{
                std::cout << "others"<< std::endl;
         }
}

void listen_cb(evconnlistener *ev,  evutil_socket_t s, sockaddr* sin, int slen, void *arg ){
        std::cout << "listen_cb"<< std::endl;
        event_base* base = (event_base*)arg;
        //创建bufferevent上下文
        //BEV_OPT_CLOSE_ON_FREE清理bufferevent时关闭socket
        bufferevent *bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
        //添加监控事件
        bufferevent_enable(bev, EV_READ |EV_WRITE);

        //设置水位
        //读取水位
        bufferevent_setwatermark(bev, EV_READ,
                5, //低水位 0就是无限制 默认0
                10 //高水位 0就是无限制
                );

        //写水位
        bufferevent_setwatermark(bev, EV_WRITE,
                5, //低水位 0就是无限制 默认0 缓冲数据低于5写入回调会被调用
                0 //写的高水位无效
        );

        //超时时间的设置
        timeval t1 = {3, 0};
        bufferevent_set_timeouts(bev, &t1, 0);
        //设置回调函数 
        bufferevent_setcb(bev, read_cb, write_cb, event_cb, base);

}

int main(int argc, char const *argv[])
{
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return 0;
        }

        event_base *base = event_base_new();
        //创建网络服务器

        //设定监听的端口和地址
        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(5001);
        
        evconnlistener *ev =  evconnlistener_new_bind(base, 
                                listen_cb,  //回调函数
                                base,  //回调函数的参数
                                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                10, //listen back
                                (sockaddr*)&sin,
                                sizeof(sin) );

        event_base_dispatch(base);
        evconnlistener_free(ev);
        event_base_free(base);
        
        return 0;
}
