#include <iostream>
#include <event2/event.h>
#include <event2/event-config.h>
#include <signal.h>
using namespace std;

/*
    @brief libevent的配置上下文
*/

static void Ctrl_C(int sock, short which, void *arg){
        std::cout << "Ctrl_C"<< std::endl;
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
        event_base_dispatch(base);

        event_free(csig);
        event_base_free(base);
        return;
}

int main(){
        test01();
        return 0;
}