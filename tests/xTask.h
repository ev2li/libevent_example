#pragma once
class xTask {
public:
        struct event_base *base = 0;
        int sock = 0;
        int thread_id = 0;

        //初始化任务
        virtual bool init() = 0;
private:
};
