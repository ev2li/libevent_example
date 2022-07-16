#pragma once
#include <vector>
#include "xThread.hpp"
#include <thread>
#include<chrono>
#include <iostream>
#include "xTask.h"

class xThreadPool{
public:
        //单例模式
        static xThreadPool* getInstance(){
                static xThreadPool p;
                return &p;
        }

        //初始化所有线程并启动线程
        void Init(int threadCount);
        
        //分发线程
        void dispatch(xTask *task);
        
private:
        int threadCount = 0; //线程数量 
        std::vector<xThread*> threads;
        xThreadPool(){};
        int lastThread = -1;
};

void xThreadPool::dispatch(xTask *task){
        if(!task) {
                return;
        }
        int tid = (lastThread + 1) % threadCount;
        std::cout << "tid:" << tid << std::endl;
        lastThread = tid;
        xThread *t = threads[tid];

        t->addTask(task);
        //激活线程
        t->activate();

}

void xThreadPool::Init(int threadCount){
        this->threadCount = threadCount;
        this->lastThread = -1;
        for (int i = 0; i < threadCount; i++) {
                xThread *t =  new xThread( i + 1);
                std::cout << "Create thread" << i + 1 << std::endl;
                t->setMid(i + 1);
                t->start();
                threads.push_back(t);
                std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
}

