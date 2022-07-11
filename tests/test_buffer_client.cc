#include <iostream>
#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/bufferevent.h>
#include <signal.h>
#include <event2/listener.h>
#include <string.h>

static int sendCount = 0;
void client_read_cb(bufferevent *be, void *arg){
        std::cout << "[client_R]" << std::endl;
        char data[1024] = {0};
        //读取输入缓冲数据
        bufferevent_read(be, data, sizeof(data) - 1);
        std::cout <<  "[" << data  << std::endl;
        bufferevent_write(be, "ok", 3);
}

void client_write_cb(bufferevent *be, void *arg){
        std::cout << "[client_W]" << std::endl;
        FILE *fp = (FILE*)arg;
        char data[1024] = {0};
        int len = fread(data, 1, sizeof(data) - 1, fp);
        if(len <= 0){
                //读到结尾或文件出错
                fclose(fp);
                //bufferevent_free(be); //立刻清理，可能会造成缓冲数据没有发送结束
                bufferevent_disable(be, EV_WRITE);
                return;
        }
        sendCount += len;
        //写入buffer
        bufferevent_write(be, data, len);
}

//错误，超时（连接断开会进入）
void client_event_cb(bufferevent *be, short events, void *arg){
         std::cout << "[client_E]" << std::endl;
         //读取超时事件发生后，数据 读取停止
         if(events & BEV_EVENT_TIMEOUT && events & BEV_EVENT_READING){
                std::cout << "BEV_EVENT_TIMEOUT"<< std::endl;
                bufferevent_free(be); //断开连接
         }else if(events & BEV_EVENT_ERROR){
                bufferevent_free(be); //断开连接
         }
        //服务端关闭事件
        if(events & BEV_EVENT_EOF ){
                std::cout << "sendCount" << sendCount << std::endl;
                std::cout << "BEV_EVENT_EOF"<< std::endl;
                bufferevent_free(be); //断开连接
        }
        if(events & BEV_EVENT_CONNECTED){
                std::cout << "BEV_EVENT_CONNECTED" << std::endl;
         }
         //触发write
         bufferevent_trigger(be, EV_WRITE, 0);
}

int main(int argc, char const *argv[])
{
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return 0;
        }

        event_base *base = event_base_new();
        //创建网络服务器

        //调用客户端代码
        //-1内部创建socket
        bufferevent *bev = bufferevent_socket_new(base,  -1, BEV_OPT_CLOSE_ON_FREE);
        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(5001);
        evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);

        FILE *fd = fopen("/var/www/source/c++/libevent_example/tests/test_buffer_client.cc", "rb");
        if(!fd){
                std::cout << "文件读取失败"<< std::endl;
                return 0;
        }
        //设置回调函数 
        bufferevent_setcb(bev, client_read_cb, client_write_cb, client_event_cb, fd);
        bufferevent_enable(bev, EV_READ|EV_WRITE);
        int rt = bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
        if(rt  == 0){
                std::cout << "connect"<< std::endl;
        }
        event_base_dispatch(base);
        event_base_free(base);
        
        return 0;
}
