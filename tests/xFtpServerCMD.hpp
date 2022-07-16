#pragma once
#include "xTask.h"
#include <event2/bufferevent.h>
#include <event2/event.h>

class xFtpServerCMD : public xTask {
public:

        xFtpServerCMD();
        virtual ~xFtpServerCMD();

        virtual bool init() override;
private:
        
};

static void read_cb(bufferevent* bev, void*arg){
        xFtpServerCMD *cmd = (xFtpServerCMD*)arg;
        char data[1024] = { 0 };
        for(;;){
                int len = bufferevent_read(bev, data, sizeof(data) -1);
                if(len <= 0){
                        break;
                }
                data[len] = '\0';
                std::cout << data<< std::flush;

                //测试代码
                if (strstr(data,"quit")){
                        bufferevent_free(bev);
                        delete cmd;
                        break;
                }
        }
}

static void event_cb(bufferevent* bev, short what, void*arg){
        xFtpServerCMD *cmd = (xFtpServerCMD*)arg;
        //如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
        if(what  &  (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)){
                std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR"<< std::endl;
                bufferevent_free(bev);
                delete cmd;
        }
}

//初始化任务，运行在子线程中
bool xFtpServerCMD::init(){
        //监听socket bufferevent
        std::cout << "xFtpServerCMD::init"<< std::endl;
        bufferevent *bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, read_cb, 0, event_cb, this);
        bufferevent_enable(bev, EV_READ|EV_WRITE);

        //添加超时
        timeval rt{10};
        bufferevent_set_timeouts(bev, &rt, 0);
        return true;
}

xFtpServerCMD::xFtpServerCMD() {
}

xFtpServerCMD::~xFtpServerCMD() {
}