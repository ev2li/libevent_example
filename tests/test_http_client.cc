#include <iostream>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/bufferevent.h>
#include <string.h>
#include <string>
#include <event2/listener.h>
#include <signal.h>

void http_client_cb(struct evhttp_request *req, void *ctx){
        std::cout << "http_client_cb"<< std::endl;

        //服务端响应错误
        if(req == NULL){
                int errcode = EVUTIL_SOCKET_ERROR();
                std::cout << evutil_socket_error_to_string(errcode)<< std::endl;
                return;
        }

        //先获取path
        const char *path = evhttp_request_get_uri(req);
        std::cout << "request path is" << path << std::endl;
        std::string filepath = ".";
        filepath += path;
        std::cout << "filepath is" << filepath<< std::endl;
        FILE *fp = fopen(filepath.c_str(), "wb");
        if(!fp){
                std::cout << "open file" << filepath << "failed" <<  std::endl;
        }
        //获取返回code 200 404
        std::cout << "Response:" << evhttp_request_get_response_code(req)<< std::endl;
        std::cout  << evhttp_request_get_response_code_line(req)<< std::endl;
        evbuffer *input =  evhttp_request_get_input_buffer(req);
        char buf[1024] = { 0 };
        for (;;) {
                int len = evbuffer_remove(input, buf, sizeof(buf) - 1);
                if(len <=0){
                        break;
                }
                buf[len] = 0;
                if(!fp){
                        continue;
                }
                fwrite(buf, 1, len, fp);
                // std::cout << buf << std::flush;
        }
}

void test(){
        //忽略管道信号，发送数据给已关闭的socket
        if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
                return;
        }

        std::cout << "test libevent client"<< std::endl;
        //创建libevent上下文
        event_base* base = event_base_new();
        if(base){
                std::cout << "event_base_new success!"<< std::endl;
        }
        //1.生成请求信息 GET
        // std::string http_url = "http://ffmpeg.club/index.html?id=1";
        std::string http_url = "http://ffmpeg.club/lesson_img/101.jpg";
        //uri
        evhttp_uri *uri =  evhttp_uri_parse(http_url.c_str());
        //http https
        const char *scheme = evhttp_uri_get_scheme(uri);
        if(!scheme){
                std::cerr << "scheme is null" << std::endl;
                return;
        }
        std::cout << "scheme is :" << scheme<< std::endl;
        //host
        const char *host = evhttp_uri_get_host(uri);
        if(!host){
                std::cerr << "host is null" << std::endl;
                return;
        }
        std::cout << "host is :" << host<< std::endl;
        //port 
        int port = evhttp_uri_get_port(uri);
 	if (port < 0){
		if (strcmp(scheme, "http") == 0){
			port = 80;
		}
	}
	std::cout << "port is " << port << std::endl;     
        //path
        const char *path = evhttp_uri_get_path(uri);
        if(!path || strlen(path) == 0){
                path = "/";
        }

        if(path){
  	        std::cout << "path is " << path << std::endl;     
        }
        //query ?后面的内容 id = 1
        const char *query = evhttp_uri_get_query(uri);
        if(query){
                 std::cout << "query is " << query << std::endl;     
        }else {
                 std::cout << "path is null" << std::endl;     
        }

        //bufferevent
        bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
         evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(
                                        base, NULL, bev, host, port);
        //http lient请求 回调函数设置
        evhttp_request *req = evhttp_request_new(http_client_cb, bev);
        //设置请求的head消息报头，信息
        evkeyvalq *output_headers  = evhttp_request_get_output_headers(req);
        evhttp_add_header(output_headers, "Host", host);
        //发起请求
        evhttp_make_request(evcon, req,  EVHTTP_REQ_GET, path);
        //事件分发处理
        if(base){
                event_base_dispatch(base);
        }
        event_base_free(base);
}

int TestGetHttp(){
	event_base* base = event_base_new();
	if (base) {
		std::cout << "event_base_new init successfuly!" << std::endl;
	}

	//http 服务器
	//1. 创建evhttp上下文
	evhttp_new(base);

	//2. 生成请求信息：GET
	std::string http_url = "http://ffmpeg.club/index.html?id=1&name=2";
	http_url = "http://ffmpeg.club/lesson_img/101.jpg";

	// 分析url地址//
	// uri
	evhttp_uri* uri = evhttp_uri_parse(http_url.c_str());
	const char* scheme = evhttp_uri_get_scheme(uri);
	if (!scheme)
	{
		std::cerr << "scheme is NULL" << std::endl;
		return -1;
	}
	std::cout << "scheme is http? =>" << scheme << std::endl;

	//port 
	int port = evhttp_uri_get_port(uri);
	if (port < 0)
	{
		if (strcmp(scheme, "http") == 0)
		{
			port = 80;
		}
	}
	std::cout << "port is " << port << std::endl;

	//host = ffmpeg.std::club
	const char* host = evhttp_uri_get_host(uri);
	if (!host)
	{
		std::cerr << "host is NULL" << std::endl;
		return -1;
	}
	std::cout << "host is ffmpeg.club? =>" << host << std::endl;

	const char* path = evhttp_uri_get_path(uri);
	if (!path || strlen(path) == 0)
	{
		path = "/";
	}
	if (path)
	{
		std::cout << "path is /index.html? =>" << path << std::endl;
	}

	//url 传参分析:?后面的内容
	const char* query = evhttp_uri_get_query(uri);
	if (query)
	{
		std::cout << "query is " << query << std::endl;
	}
	else
	{
		std::cout << "query is NULL" << std::endl;
	}

	//bufferevent 连接http服务器
	bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	evhttp_connection* ev_conn = evhttp_connection_base_bufferevent_new(base, NULL, bev, host, port);

	//http client 请求 回调函数设置
	evhttp_request* req = evhttp_request_new(http_client_cb, base);

	//设置请求header信息
	evkeyvalq* output_header = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_header, "Host", host);

	//发起请求
	evhttp_make_request(ev_conn, req, EVHTTP_REQ_GET, path);

	// 事件分发处理
	if (base) {
		event_base_dispatch(base);
	}
	if (base) {
		event_base_free(base);
	}
        return 0;
}

int TestPostHttp() {
	event_base* base = event_base_new();
	if (base) {
		std::cout << "event_base_new init successfuly!" << std::endl;
	}

	//http 服务器
	//1. 创建evhttp上下文
	evhttp_new(base);

	//2. 生成请求信息：GET
	std::string http_url = "http://192.168.140.139:8080/index.html";

	// 分析url地址//
	// uri
	evhttp_uri* uri = evhttp_uri_parse(http_url.c_str());
	const char* scheme = evhttp_uri_get_scheme(uri);
	if (!scheme)
	{
		std::cerr << "scheme is NULL" << std::endl;
		return -1;
	}
	std::cout << "scheme is http? =>" << scheme << std::endl;

	//port 
	int port = evhttp_uri_get_port(uri);
	if (port < 0)
	{
		if (strcmp(scheme, "http") == 0)
		{
			port = 80;
		}
	}
	std::cout << "port is " << port << std::endl;

	//host = ffmpeg.club
	const char* host = evhttp_uri_get_host(uri);
	if (!host)
	{
		std::cerr << "host is NULL" << std::endl;
		return -1;
	}
	std::cout << "host is ffmpeg.club? =>" << host << std::endl;

	const char* path = evhttp_uri_get_path(uri);
	if (!path || strlen(path) == 0)
	{
		path = "/";
	}
	if (path)
	{
		std::cout << "path is /index.html? =>" << path << std::endl;
	}

	//url 传参分析:?后面的内容
	const char* query = evhttp_uri_get_query(uri);
	if (query)
	{
		std::cout << "query is " << query << std::endl;
	}
	else
	{
		std::cout << "query is NULL" << std::endl;
	}

	//bufferevent 连接http服务器
	bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	evhttp_connection* ev_conn = evhttp_connection_base_bufferevent_new(base, NULL, bev, host, port);

	//http client 请求 回调函数设置
	evhttp_request* req = evhttp_request_new(http_client_cb, base);

	//设置请求header信息
	evkeyvalq* output_header = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_header, "Host", host);

	//发送post数据
	evbuffer* output = evhttp_request_get_output_buffer(req);
	evbuffer_add_printf(output, "test_post_request=%d&name=%d", 1, 2);

	//发起请求
	evhttp_make_request(ev_conn, req, EVHTTP_REQ_POST, path);

	// 事件分发处理
	if (base) {
		event_base_dispatch(base);
	}
	if (base) {
		event_base_free(base);
	}
        return 0;
}

int main(){
//    test();
        TestGetHttp();
        std::cout << "========== Test TestGetHttp client! ========" << std::endl;

        TestPostHttp();
        std::cout << "========== Test TestPostHttp client! ========" << std::endl;
   return 0;
}