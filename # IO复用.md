# IO复用
## 目的
解决状态服务器接收多个请求的问题
## 可选方法
1. 把所有的描述符设置为**非阻塞模式**，导致CPU占用过高 //TODO
2. 使用信号（singal），当文件描述符状态变化时调用信号处理函数。**fcntl(F_SETSIG)**设置的极不合理
3. 系统提供一个方法告诉哪个描述符就绪了，（`select``poll``epoll`都是这种模式）
4. 引入一个**进程寄存器(process register)**去记录OS中所有的（感兴趣的）事件，（`Windows kqueue`是这种模式）

### 信号驱动
#### 优点
1. 性能好
#### 缺点
1. 编程复杂
### Select poll()
#### 优点
1. 可移植
#### 缺点
1. 线性扫描，任何的调用都会触发对IO集的扫描
2. 不好被多个进程共享
#### epoll的解决方法
1. 引入epoll_wait
2. 引入EPOLLEXCLUSIVE标志解决鸟群现象
### Epoll()
#### 优点
1. 对很多文件描述符监控会有好的性能
#### 缺点
1. 是Linux的API

- 设计上的
1. 不支持多线程（后引入EPOLLONESHOT标志）
2. 引入了中间层
	epoll_ctl(EPOLL_CTL_ADD) doesn't actually register a file descriptor. Instead it registers a tuple1 of a file descriptor and a pointer to underlying kernel object. Most confusingly the lifetime of an epoll subscription is not tied to the lifetime of a file descriptor. It's tied to the life of the kernel object.
	You always must always explicitly call epoll_ctl(EPOLL_CTL_DEL) before calling close().


3. 鸟群现象EPOLLEXCLUSIVE
4. 数据竞争EPOLLONESHOT
