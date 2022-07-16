#pragma once
#include "xFtpTask.hpp"
#include <iostream>
class xFtpUSER : public xFtpTask {
public:
        virtual void parser(std::string type, std::string msg);
        xFtpUSER();
        ~xFtpUSER();

private:
        
};

xFtpUSER::xFtpUSER(){

}

xFtpUSER::~xFtpUSER(){

}

void xFtpUSER::parser(std::string type, std::string msg){
        std::cout << "xFtpUser::parser" << type << " " << msg<< std::endl;
        resCMD("230 Login successful.\n");
}