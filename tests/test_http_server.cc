#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <signal.h>
#include <memory.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <string>
#include <stdio.h>
#define WEBROOT  "."
#define DEFAULTINDEX "index.html"
void http_cb(evhttp_request *request, void *arg){
        std::cout << "http_cb"<< std::endl;
        //1.获取浏览器的请求信息
        //uri获取
        const char *uri = evhttp_request_get_uri(request);
        std::cout << uri << std::endl;
        //获取请求类型 GET POST
        std::string cmdtype;
        switch (evhttp_request_get_command(request)) {
                case EVHTTP_REQ_GET :
                        cmdtype = "GET";
                        break;
                case EVHTTP_REQ_POST:
                        cmdtype = "POST";
                        break;
                default:
                        break;
        }
        std::cout << "cmdtype:" << cmdtype << std::endl;
        //消息报头
        evkeyvalq *headers = evhttp_request_get_input_headers(request);
        std::cout << "==============headers==============="<< std::endl;
        for (evkeyval *p = headers->tqh_first;  p != NULL; p = p->next.tqe_next) {
                std::cout << p->key << ":" << p->value<< std::endl;
        }
        //获取请求正文(GET为空,POST有表单信息)
        evbuffer *inbuf = evhttp_request_get_input_buffer(request);
        char buf[1024] = {0};
        std::cout << "================input data============"<< std::endl;
        while (evbuffer_get_length(inbuf) ) {
                int  n =  evbuffer_remove(inbuf, buf, sizeof(buf) - 1);
                if( n > 0 ){
                        buf[n] = '\0';
                        std::cout << buf<< std::endl;
                }
        }
        //2.回复浏览器
        //状态行，消息报头 响应正文
        //分析出请求的文件uri
        //设置根目录WEBROOT
        std::string filepath = WEBROOT;
        filepath += uri;
        if(strcmp(uri, "/") == 0){
                filepath += DEFAULTINDEX;
        }
        //消息报头
        evkeyvalq *outhead = evhttp_request_get_output_headers(request); 
        //要支持图片 js css下载zip文件
        //先获取文件的后缀名 /root/index.html
        int pos = filepath.rfind('.');
        std::string postfix = filepath.substr(pos + 1, filepath.size() - (pos + 1));
        std::cout << postfix<< std::endl;
        if(postfix  == "jpg" || postfix == "gif" || postfix == "png"){
                std::string tmp = "image/" + postfix;
                evhttp_add_header(outhead, "Content-Type", tmp.c_str());
        }else if (postfix == "zip"){
                 evhttp_add_header(outhead, "Content-Type", "application/zip");
        }else if (postfix == "html"){
                 evhttp_add_header(outhead, "Content-Type", "text/html");
        }else if (postfix == "css"){
                 evhttp_add_header(outhead, "Content-Type", "text/css");
        }else if (postfix == "js"){
                 evhttp_add_header(outhead, "Content-Type", "text/js");
        }
    
        

        std::cout << filepath.c_str()<< std::endl;
        //读取html文件返回正文
        FILE *fp = fopen(filepath.c_str(), "rb");
        if(!fp){
                 evhttp_send_reply(request, HTTP_NOTFOUND,  "",  0);
                 return;
        }

        evbuffer *outbuf = evhttp_request_get_output_buffer(request);
        for (;;) {
                int len = fread(buf, 1, sizeof(buf), fp);
                if(len <= 0){
                        break;
                }
                evbuffer_add(outbuf, buf, len);
        }
        fclose(fp);           
        evhttp_send_reply(request, HTTP_OK,  "",  outbuf);    

}

void test(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }

        std::cout << "test http server"<< std::endl;
        //创建libevent上下文
        event_base* base = event_base_new();
        if(base){
                std::cout << "event_base_new success"<< std::endl;
        }

        //http服务器
        //1.创建evhttp上下文
        evhttp *evh = evhttp_new(base);
        
        //2.绑定端口
       if(evhttp_bind_socket(evh, "0.0.0.0", 8080) != 0){
                std::cout << "evhttp_bind_socket failed!"<< std::endl;
       } 

        //3.设定回调函数
        evhttp_set_gencb(evh, http_cb, 0);

        //事件分发处理
        if(base){
                event_base_dispatch(base);
        }
        
        if(base){
                event_base_free(base);
        }

        if(evh){
                evhttp_free(evh);
        }
}

int main(){
   test();
   return 0;
}