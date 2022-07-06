#include <iostream>
#include <event2/event.h>
#include <event2/event-config.h>
#include <signal.h>
using namespace std;

/*
    @brief libevent的配置上下文
*/

void test01(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }



        //配置上下文
        event_config *conf = event_config_new();   
#if 0
        //显示支持的网络模式
        const char** methods = event_get_supported_methods();
        std::cout << "supported_methods:"<< std::endl;
        for (size_t i = 0; methods[i] != NULL;  i++) {
                std::cout << methods[i]<< std::endl;
        }
#endif
        //设置特征，
        // event_config_require_features(conf, EV_FEATURE_FDS); //设置完这个epoll就不能用了
 #if 0       
        //设置网络模型，使用select
        event_config_avoid_method(conf, "epoll");
        event_config_avoid_method(conf, "poll");
#endif
        //初始化配置libevent上下文
        event_base *base = event_base_new_with_config(conf);
        event_config_free(conf);

          
        if(!base){
                std::cout << "event_base_new_with_config failed!"<< std::endl;
                base = event_base_new();
                if(base){
                        std::cerr << "event_base_new failed!" << std::endl;
                }
                return;
        }else {
                //获取当前网络模型
                std::cout <<  event_base_get_method(base) << std::endl;

#if 0
                //确认特征是否生效
                int f = event_base_get_features(base);
                if(f & EV_FEATURE_ET ){
                        std::cout << "EV_FEATURE_ET events are supported.\n";
                }else {
                        std::cout << "EV_FEATURE_ET events are not supported.\n";
                }

                if(f & EV_FEATURE_O1 ){
                        std::cout << "EV_FEATURE_O1 events are supported.\n";
                }else {
                        std::cout << "EV_FEATURE_O1 events are not supported.\n";
                }

                 if(f & EV_FEATURE_FDS ){
                        std::cout << "EV_FEATURE_FDS events are supported.\n";
                }else {
                        std::cout << "EV_FEATURE_FDS events are not supported.\n";
                }

                if(f & EV_FEATURE_EARLY_CLOSE ){
                        std::cout << "EV_FEATURE_EARLY_CLOSE events are supported.\n";
                }else {
                        std::cout << "EV_FEATURE_EARLY_CLOSE events are not supported.\n";
                }
#endif
                std::cout << "event_base_new_with_config success!"<< std::endl;
                event_base_free(base);
        }
}

int main(){
        test01();
        return 0;
}