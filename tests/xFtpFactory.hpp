#pragma once
#include "xTask.h"
#include "xFtpServerCMD.hpp"
#include "xFtpUSER.hpp"
#include "xFtpLIST.hpp"
#include "xFtpPORT.hpp"
#include "xFtpRETR.hpp"
class xFtpFactory {
public:
        static xFtpFactory *getInstance(){
                static xFtpFactory f;
                return &f;
        }

        xTask* createTask();
private:
        xFtpFactory();
};


xTask* xFtpFactory::createTask(){
        xFtpServerCMD *x = new xFtpServerCMD();
        //注册ftp消息处理对象
        x->reg("USER", new xFtpUSER());
        xFtpLIST *list = new xFtpLIST();
        x->reg("PWD", list);
        x->reg("LIST", list);
        x->reg("CWD", list);
        x->reg("CDUP", list);
        x->reg("PORT", new xFtpPORT());
        x->reg("RETR", new xFtpRETR());
        return x;
}

xFtpFactory::xFtpFactory(){

}