#pragma once
#include "xTask.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <iostream>
#include <string>

class xFtpTask :public xTask{
public:
        //解析协议
        virtual void parser(std::string type, std::string msg){}

        //回复cmd消息
        void resCMD(std::string msg);
        
        //连接数据通道
        void connectPORT();
        //用来发送建立了连接的数据通道
        void send(std::string msg);
        void send(const char* data, int datasize);
        void close();

        virtual void read(struct bufferevent *bev){};
        virtual void write(struct bufferevent *bev){};
        virtual void event(struct bufferevent *bev, short what){};
        
        void setCallback(struct bufferevent *bev);
        virtual bool init() override;

        //PORT 数据通道的IP和端口
        std::string curDir = "/";
        std::string rootDir = ".";
        //命令通道
        xFtpTask *cmdTask = 0;
        //命令bev
        struct bufferevent *bev = 0;
        std::string ip = "";
        int port = 0;
protected:
        static void read_cb(struct bufferevent *bev, void *arg);
        static void write_cb(struct bufferevent *bev, void *arg);
        static void event_cb(struct bufferevent *bev, short what, void *arg);
};

void xFtpTask::read_cb(struct bufferevent *bev, void *arg){
        xFtpTask *t = static_cast<xFtpTask*>(arg);
        t->read(bev);
}

void xFtpTask::write_cb(struct bufferevent *bev, void *arg){
        xFtpTask *t = static_cast<xFtpTask*>(arg);
        t->write(bev);
}

void xFtpTask::event_cb(struct bufferevent *bev, short what, void *arg){
        xFtpTask *t = static_cast<xFtpTask*>(arg);
        t->event(bev, what);
}

void xFtpTask::setCallback(struct bufferevent *bev){
        bufferevent_setcb(bev, read_cb, write_cb, event_cb, this);
        bufferevent_enable(bev, EV_READ|EV_WRITE);
}

bool xFtpTask::init(){
        return true;
}

void xFtpTask::resCMD(std::string msg){
        if(!cmdTask || !cmdTask->bev){
                return;
        }
        std::cout << "resCMD:" << msg << std::endl;
        bufferevent_write(cmdTask->bev, msg.c_str(), msg.size());
}
void xFtpTask::connectPORT(){
        if(ip.empty() || port <= 0  || !base){
                std::cout << "connectPORT failed ip or port or base is null"<< std::endl;
                return;
        }
        close();
        bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(port);
        evutil_inet_pton(AF_INET, ip.c_str(), &sin.sin_addr.s_addr);

        //设置回调和权限
        setCallback(bev);
        //添加超时
        timeval rt = {60, 0};
        bufferevent_set_timeouts(bev, &rt, 0);
        bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
}


void xFtpTask::send(std::string msg){
        send(msg.c_str(), msg.size());
}

void xFtpTask::send(const char* data, int datasize){
        if( !bev){
                return;
        }
        bufferevent_write(bev, data, datasize);
}

void xFtpTask::close(){
        if(bev){
                bufferevent_free(bev);
                bev = 0;
        }
}
