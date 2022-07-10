# libevent_example
## 环境要求
libevent学习
zlib1.2.11
openssl1.1.1源码
libevent2.1.8源码

## epoll相关
- LT(Level Triggered): 事件没处理，一直通知 (默认)
- ET(Edge Triggered)：只通知一次，每当状态变化时，触发一个事件（EV_ET）

## 主要的api
- event_base_new()
- event_base_config()
- event_base_new_with_config()
- event_add()
- event_new()
- event_base_dispatch()
- event_base_free()
- event_config_free()

## epoll
- evconnlistener_new_bind()
- evconnlistener_free()