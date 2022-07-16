#pragma once
#include "xFtpTask.hpp"
#include <string>
#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>

class xFtpLIST : public xFtpTask{
public:
        xFtpLIST();
        ~xFtpLIST();
        //解析协议
        virtual void parser(std::string type, std::string msg);
        virtual void write(struct bufferevent *bev);
        virtual void event(struct bufferevent *bev, short what);
private:
        
};

xFtpLIST::xFtpLIST() {
}

xFtpLIST::~xFtpLIST() {
}

//解析协议
void xFtpLIST::parser(std::string type, std::string msg){
        std::string resmsg = "";

        if(type == "PWD"){
                resmsg = "257 \"";
                resmsg += cmdTask->curDir;
                resmsg += "\" is current dir.";
                resCMD(resmsg);
        }else  if(type == "LIST"){
                // 1 连接数据通道 2 150 3 发送数据通道 4 发送完成226 5关闭连接 
                //回复消息，使用数据通道发送目录
                //1.连接数据通道
                connectPORT();
                resCMD("150 Here comes the directory listing.\r\n");
                std::string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n";
                //数据通道发送
                send(listdata);

        }
}


void xFtpLIST::write(struct bufferevent *bev){
        //4 发送完成
        resCMD("226 Transfer complete\r\n");
        //5 关闭连接
        close();
}

void xFtpLIST::event(struct bufferevent *bev, short what){
        //如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
        if(what  &  (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)){
                std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR"<< std::endl;
                close();
        }else if(what & BEV_EVENT_CONNECTED) {
                std::cout << "xFtpLIST BEV_EVENT_CONNECTED"<< std::endl;
        }
}