#pragma once
#include "xFtpTask.hpp"
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <iostream>
#include <string>
#include <map>

class xFtpServerCMD : public xFtpTask {
public:

        xFtpServerCMD();
        virtual ~xFtpServerCMD();
        void read(struct bufferevent *bev);
        void event(struct bufferevent *bev, short what);
        bool init() override;
        //注册命令处理对象不需要考虑线程安全，在调用时还未分发到线程
        void reg(std::string, xFtpTask *call);
private:
        std::map<std::string, xFtpTask*> calls; 
};

void xFtpServerCMD::reg(std::string cmd, xFtpTask *call){
        if(!call){
                std::cout << "xFtpServerCMD::reg call is null"<< std::endl;
                return;
        }

        if(cmd.empty()){
                std::cout << "xFtpServerCMD::reg cmd is null"<< std::endl;
                return;
        }

        //已经注册的是否覆盖(约定不覆盖，提示错误)
        if(calls.find(cmd) != calls.end()){
                std::cout << cmd  << " is alreay register"<< std::endl;
                return;               
        }

        calls[cmd] = call;
}

void xFtpServerCMD::read(bufferevent* bev){
        char data[1024] = { 0 };
        for(;;){
                int len = bufferevent_read(bev, data, sizeof(data) -1);
                if(len <= 0){
                        break;
                }
                data[len] = '\0';
                std::cout << "recv cmd:" << data << std::flush;
                //分发到处理对象
                //分析出类型 USER anonymous
                std::string type = "";
                for (int i = 0; i < len; i++) {
                        if(data[i] == ' ' ||  data[i] == '\r'){
                                break;
                        }
                        type += data[i];
                }
                std::cout << "type is:[" << type  << "]"<< std::endl;
                if(calls.find(type)  != calls.end() ){
                        xFtpTask *t = calls[type];
                        t->cmdTask = this; //用来处理回复命令和目录
                        ip = t->ip;
                        port = t->port;
                        t->base = base;
                        t->parser(type, data);
                        if(type == "PORT"){
                                std::cout << "ssssssssssss"<< std::endl;
                                ip = t->ip;
                                port = t->port;
                        }
                }else {
                        std::string msg = "200 OK\n";
                        bufferevent_write(bev, msg.c_str(), msg.size());
                }
        }
}

void xFtpServerCMD::event(bufferevent* bev, short what){
        //如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
        if(what  &  (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)){
                std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR"<< std::endl;
                bufferevent_free(bev);
                delete this;
        }
}

//初始化任务，运行在子线程中
bool xFtpServerCMD::init(){
        //监听socket bufferevent
        std::cout << "xFtpServerCMD::init"<< std::endl;
        bufferevent *bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
        this->bev = bev;
        this->setCallback(bev);

        //添加超时
        timeval rt{60, 0};
        bufferevent_set_timeouts(bev, &rt, 0);
        std::string msg = "220 Welcome to libevent xFtpServer\n";
        bufferevent_write(bev, msg.c_str(), sizeof(msg));
        return true;
}

xFtpServerCMD::xFtpServerCMD() {
}

xFtpServerCMD::~xFtpServerCMD() {
}