#pragma once
#include <thread>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <event2/event.h>
#include <list>
#include <mutex>
#include "xTask.h"

class xThread{
public:
        /**
         * @brief  启动线程
         * 
         */
        void start();
        //线程入口函数
        void mainFunc();
        //安装线程，初始化event_base和管道监听事件用于激活线程
        bool setUp();
        void notify(evutil_socket_t fd, short which); //收到主线程发出的激活信息（线程池的分发）
        //线程激活
        void activate();
        //添加处理任务，一个线程同时可以处理多个任务，共用一个event base
        void addTask(xTask *t);
        xThread(int id);
        ~xThread();
        void setMid(int v) { m_id = v; }
private:
        int m_id = 0; //线程编号
        int notify_send_fd = 0;
        event_base* base;
        //任务列表
        std::list<xTask*> tasks;
        //线程安全
        std::mutex tasks_mutex;
};


void xThread::addTask(xTask *t){
        if(!t) return;
        t->base = this->base;
        tasks_mutex.lock();
        tasks.push_back(t);
        tasks_mutex.unlock();
}

//激活线程任务的事件回调函数
static void notify_cb(evutil_socket_t fd, short which, void*arg){
	xThread* t = (xThread*)arg;
	t->notify(fd, which);
}


void xThread::start(){
        setUp();
        std::thread th(&xThread::mainFunc, this);
        //断开与主线程联系
        th.detach();
}

void xThread::mainFunc(){
        std::cout << m_id << " xThread::mainFunc() begin" << std::endl;
        event_base_dispatch(base);
        event_base_free(base);
        std::cout << m_id << " xThread::mainFunc() end" << std::endl;
}

xThread::xThread(int id)
        :m_id(id){
}

bool xThread::setUp(){
        //对管道用read write
        int fds[2];
        if(pipe(fds)){
                std::cout << "pipe failed!"<< std::endl;
                return false;
        }

        //读取绑定到event事件中，写入要保存。
	notify_send_fd = fds[1];
        //创建libevent上下文(无锁)
        event_config *ev_config  = event_config_new();
        event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);
        this->base = event_base_new_with_config(ev_config);
        if(!base){
                std::cerr << "event_base_new_with_config failed in thread" << std::endl;
                return false;
        }

        //添加管道监听事件，用于激活线程执行任务
        event *ev = event_new(base, fds[0], EV_READ|EV_PERSIST, notify_cb, this);
        event_add(ev, 0);
        event_config_free(ev_config);
        return true;
}

xThread::~xThread(){
        
}


void xThread::notify(evutil_socket_t fd, short which){
        char buf[2] = { 0 };
        int rt = read(fd, buf, 1);
        if(rt <= 0){
                return;
        }
        std::cout << m_id << " thread" << buf << std::endl;
        xTask *task = nullptr;
        //获取任务,并初始化任务
        tasks_mutex.lock();
        if(tasks.empty()){
                tasks_mutex.unlock();
                return;
        }
        //先进先出
        task = tasks.front();
        tasks.pop_front();
        tasks_mutex.unlock();
        task->init();
}

void xThread::activate(){
        int rt = write(this->notify_send_fd,  "c", 1);
        if(rt <= 0){
                std::cerr << "xThread::activate() failed!" << std::endl;
        }
}