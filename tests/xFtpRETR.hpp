#pragma once
#include "xFtpTask.h"
#include <string>
#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>

class xFtpRETR : public xFtpTask{
public:
        xFtpRETR();
        ~xFtpRETR();
        virtual void parse(std::string type, std::string msg);
        virtual void write(struct bufferevent *bev);
        virtual void event(struct bufferevent *bev, short what);
private:
        FILE *fp = 0;
        char buf[1024] = { 0 };
};

xFtpRETR::xFtpRETR() {
}

xFtpRETR::~xFtpRETR() {
}

void xFtpRETR::parse(std::string type, std::string msg){
        //取出文件名
        int pos = msg.rfind(" ") + 1;
        std::string filename = msg.substr(pos, msg.size() - pos -1);
        std::string path = cmdTask->rootDir;
        path += cmdTask->curDir;
        path += filename;
        fp = fopen(path.c_str(), "rb");
        if(fp){
                //连接数据通道
                connectPORT();
                resCMD("150 File OK!\n");
                //发送开始下载文件的指令
                //触发写入事件
                bufferevent_trigger(bev, EV_WRITE, 0);
        }else {
                resCMD("450 File open failed!\n");
        }
}


void xFtpRETR::write(struct bufferevent *bev){
        if(!fp){
                return;
        }

        int len = fread(buf, 1, sizeof(buf), fp);
        if(len <= 0){
                fclose(fp);
                fp = 0;
                resCMD("226 Transfer complete\n");
                Close();
                return;
        }
        std::cout << "[" << len << "]" << std::flush;
        send(buf, len);
};

void xFtpRETR::event(struct bufferevent *bev, short what){
        //如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
        if(what  &  (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)){
                std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR"<< std::endl;
                close();
                if(fp){
                        fclose(fp);
                        fp = 0;
                }
        }else if(what & BEV_EVENT_CONNECTED) {
                std::cout << "xFtpLIST BEV_EVENT_CONNECTED"<< std::endl;
        }
};