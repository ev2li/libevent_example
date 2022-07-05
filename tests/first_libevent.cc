#include <iostream>
#include <event2/event.h>
using namespace std;

int main(){
        std::cout << "test libevent"<< std::endl;
        event_base* base = event_base_new();
        if(base){
                std::cout << "event_base_new success!" << std::endl;
        }
        return 0;
}