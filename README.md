# Which is the best IO model
We aimed to provide:
* Minimal implementations of all kinds of IO model for network library, both client and server. 
* Minimal interfaces for users of network library. 
* Minimal implementations of different protocols.
## TODO
- [ ] 对服务端不同IO模型进行测试
- [ ] 对客户端不同IO模型进行测试
- [ ] 支持RDMA
- [ ] 支持buffer size配置
## 路线
### 单线程
- [x]  阻塞IO, 同步业务逻辑
- [ ]  非阻塞IO, 异步业务逻辑
  - [ ] server应当提供异步的IO接口支持业务逻辑: connect, send, recv
- [ ]  非阻塞IO, cpp20协程+同步业务逻辑
  - [ ] server应当提供awaitable的send/recv接口支持业务逻辑
### 多线程
TODO