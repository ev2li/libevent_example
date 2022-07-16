#pragma once 
#include "xFtpTask.hpp"
#include <vector>
#include <iostream>
#include <string>

class xFtpPORT : public xFtpTask{
public:
        virtual void parser(std::string type, std::string msg);
        xFtpPORT();
        ~xFtpPORT();

private:

};

xFtpPORT::xFtpPORT() {
}

xFtpPORT::~xFtpPORT() {
}

void xFtpPORT::parser(std::string type, std::string msg){
        //PORT 127,0,0,1,70,96 \r\n
        //PORT n1,n2,n3,n4,n5,n6\r\n
        //port = n5 * 256 + n6

        //只获取ip和端口，不连接
        //取出ip
        std::vector<std::string> vals;
        std::string tmp = "";
        for (size_t i = 5; i < msg.size(); i++) {
                if(msg[i] == ','   || msg[i] == '\r' ){
                        vals.push_back(tmp);
                        tmp = "";
                        continue;
                }
                tmp += msg[i];
        }
        if(vals.size()  !=   6){
                //PORT 格式有误
                resCMD("501 Syntax error in parameters or arguments");
                return;
        }

        // for (size_t i = 0; i < vals.size(); i++) {
        //         std::cout << vals[i]<< std::endl;
        // }
        ip = vals[0] + "." + vals[1] + "." + vals[2] + "." + vals[3];
        port  = atoi(vals[4].c_str()) * 256 + atoi(vals[5].c_str());
        std::cout << "PORT ip is:" << ip<< std::endl;
        std::cout << "PORT port is:" << port << std::endl;
        resCMD("200 PORT command successful.\r\n");
}