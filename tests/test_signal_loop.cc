#include <iostream>
#include <event2/event.h>
#include <event2/event-config.h>
#include <signal.h>
using namespace std;
bool isexit = false;
/*
    @brief libevent的配置上下文
*/

static void Ctrl_C(int sock, short which, void *arg){
        std::cout << "Ctrl_C"<< std::endl;
        event_base *base = (event_base*)arg;
        //执行完当前处理的事件函数就退出
        // event_base_loopbreak(base);

        //运行完所有的活动事件再退出 事件循环没有运行时等运行一次再退出
        timeval t = {3, 0}; //至少运行三秒退出
        event_base_loopexit(base, &t);
}

static void Kill(int sock, short which, void *arg){
        std::cout << "kill"<< std::endl;
        event *ev = (event *)arg;
        if(!event_pending(ev, EV_SIGNAL, NULL)){ //如果处于非待决状态
                event_del(ev);
                event_add(ev, NULL);
        }
}

void test01(){
        event_base *base = event_base_new();
        //添加信号ctrl_c 信号事件，处于no pending
        //evsignal_new隐藏的状态 EV_SIGNAL |EV_PERSIST
        event *csig = evsignal_new(base, SIGINT, Ctrl_C, base);
        if(!csig){
                std::cerr << "SIGINT evsignal_new failed!"<< std::endl;
        }
        //添加事件到pending
        if(event_add(csig,0) != 0){
                std::cerr << "SIGINT evsignal_add failed!"<< std::endl;
        }
        
        //添加kill信号(非持久，只进入一次)event_self_cbarg 传递当前的event
        event *ksig = event_new(base, SIGTERM,EV_SIGNAL, Kill, event_self_cbarg());
        //添加事件到pending
        if(event_add(ksig,0) != 0){
                std::cerr << "SIGINT evsignal_add failed!"<< std::endl;
        }
        
        //进入事件主循环
        // event_base_dispatch(base);
         //EVLOOP_ONCE等待事件运行，直到没有活动事件就退出返回0
        // event_base_loop(base, EVLOOP_ONCE);

        //EVLOOP_NONBLOCK 有活动事件就处理，没有就返回0
        /*while(!isexit){
                event_base_loop(base, EVLOOP_NONBLOCK);
        }*/
        //EVLOOP_NO_EXIT_ON_EMPTY 没有注册事件也不返回（用于事件后期添加）
        event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
        event_free(csig);
        event_base_free(base);
        return;
}

int main(){
        test01();
        return 0;
}