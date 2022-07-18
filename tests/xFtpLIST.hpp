#pragma once
#include "xFtpTask.hpp"
#include <string>
#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <io.h>

class xFtpLIST : public xFtpTask{
public:
        xFtpLIST();
        ~xFtpLIST();
        //解析协议
        virtual void parser(std::string type, std::string msg);
        virtual void write(struct bufferevent *bev);
        virtual void event(struct bufferevent *bev, short what);
private:
        std::string getListData(std::string path);
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
                // std::string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n";
               std::string listdata = getListData(cmdTask->rootDir + cmdTask->curDir);
                //数据通道发送
                send(listdata);

        }else if (type == "CWD"){ //切换目录
                //取出命令中的路径
                int pos = msg.rfind(" ") + 1;
                std::string path = msg.substr(pos, msg.size() - pos - 1); //去掉结尾的 /n  
                if(path[0] ==  '/' ){ //绝对路径
                        cmdTask->curDir = path;
                }else{
                        if(cmdTask->curDir[cmdTask->curDir.size() - 1] != '/'){
                                cmdTask->curDir  += "/";
                        }
                        cmdTask->curDir += path + "/";
                }
                resCMD("250 Directory success chanaged! \r\n");
        }else if (type == "CDUP"){
                std::string path = cmdTask->curDir;
                //统一去掉结尾的"/"
                if(path[path.size() - 1] == '/' ){
                        path = path.substr(0, path.size() - 1);
                }

                int pos = pos.rfind("/");
                path = path.substr(0, pos);
                cmdTask->curDir = path;
                resCMD("250 Directory success chanaged! \r\n");
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

std::string xFtpLIST::getListData(std::string){
        //-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
        std::string data = "";
        //存储文件信息
        _finddata_t file;
        //目录上下文
        path += "/*.*"
        intptr_t dir = _findfirst(path.c_str(), &file);
        if(dir < 0 ){
                return data;
        }
        do {
                std::string tmp = "";
                //判断是否是目录 去掉.和..
                if(file.attrib & _A_SUBDIR){
                        if(strcmp(file.name, ".") == 0  || strcmp(file.name, "..") == 0 ){
                                continue;
                        }
                        tmp = "drwxrwxrwx 1 root group ";
                }else {
                        tmp = "-rwxrwxrwx 1 root group ";
                }
                //文件大小
                char buf[1024];
                sprintf(buf, "%u ", file.size);
                tmp += buf;
                //日期
                strftime(buf, sizeof(buf) - 1, "%b %m %H:%M", localtime(&file, time_write));
                tmp += buf;
                tmp += file.name
                data += "\n";
                data += tmp;
        } while (_findnext(dir,&file) == 0);
        return data;
}
