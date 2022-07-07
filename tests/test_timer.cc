#include <iostream>
#include <event2/event.h>
#include <signal.h>

using namespace std;

/*
    @brief
*/
static timeval t1 = {1, 0 };
void timer1(int sock, short which, void *arg){
        std::cout << "[timer]"<< flush;
        event *ev = (event *)arg;
        //no pending
        if(!evtimer_pending(ev, &t1)){
        evtimer_del(ev);
        evtimer_add(ev, &t1);
        }
}

void timer2(int sock, short which, void *arg){
        std::cout << "[timer 2]"<< flush;
}

void timer3(int sock, short which, void *arg){
        std::cout << "[timer 3]"<< flush;
}

void test01(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }
        event_base* base = event_base_new();

        //定时器
        std::cout << "test timer"<< std::endl;
        // event* ev = evtimer_new( base, timer1 , event_self_cbarg());
        //定时器，非持久事件
       /* event* ev = evtimer_new( base, [](int sock, short which, void *arg){
                 std::cout << "[timer]"<< flush;
                 event *ev = (event *)arg;
                 //no pending
                 if(!evtimer_pending(ev, &t1)){
                        evtimer_del(ev);
                        evtimer_add(ev, &t1);
                 }
        }, event_self_cbarg());*/
        static timeval t2;
        t2.tv_sec = 1;
        t2.tv_usec = 200000; //微秒
        event *ev = event_new(base, -1, EV_PERSIST, timer2, 0);
        if(!ev){
                std::cout << "evtimer_new failed!"<< std::endl;
                return;
        }
        event_add(ev, &t2); //插入性能O(logn)

        //定时器超时优化，默认event是用二叉堆存储（完全二叉树），插入删除O(logn)
        //优化到双向队列，插入删除O(1)
        static timeval tv_in = {3, 0};
        const timeval *t3;
        t3 = event_base_init_common_timeout(base, &tv_in);
         event *ev3 = event_new(base, -1, EV_PERSIST, timer3, 0);
        event_add(ev3, t3); //插入性能O(1)
        //主循环
        event_base_dispatch(base);
        event_base_free(base);
        return;
}

int main(){
        test01();
        return 0;
}