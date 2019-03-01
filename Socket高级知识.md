[toc]
# Socket高级知识
## 流Socket的部分读
部分读意思是`read(), write()`系统调用没有读够需要长度的字节数。以下情况发生时会发生部分读：  
1. 内核Buffer不够
2. `write()`被信号中断
3. `socket`以**O_NONBLOCK**模式打开
4. **异步错误**发生，比如对方的连接断开

部分IO并不是错误，只要读写了1个字节都算成功。

### 处理部分IO的函数
```c
#include "rdwrn.h"
ssize_t readn(int fd, void *buffer, size_t count);
Returns number of bytes read, 0 on EOF, or –1 on error
ssize_t writen(int fd, void *buffer, size_t count );
Returns number of bytes written, or –1 on erro
```
具体实现:在书的*Listing 61-1*

## shutdown()系统调用
```c
#include <sys/socket.h>
int shutdown(int sockfd , int how);
Returns 0 on success, or –1 on erro
```
关闭单向或者双向连接，具体取决于`how`参数。

| 参数值    | 行为                                                                                        |
|-----------|---------------------------------------------------------------------------------------------|
| SHUT_RD   | 关闭读（读返回**EOF**），可以写，连接对方会收到**SIGPIPE**信号，写的时候会产生**EPIPE**错误 |
| SHUT_WR   | 关闭写（写会收到**SIGPIPE**信号，产生**EPIPE**错误），可以读，应用于`ssh, rsh`程序          |
| SHUT_RDWR | 关闭读写                                                                                    |



`shutdown()`针对的是`file description`而非`file descriptor`，因此对文件描述符的`shutdown()`会影响所有指向此`file description`的`file descriptor`。而相对的`close()`针对的是`file descriptor`，然而由于`shutdown()`不能关闭文件描述符，所以总是得调用`close()`去关闭`file descriptor`。


## Socket特定IO系统调用 recv(), send()
```c
#include <sys/socket.h>
ssize_t recv(int sockfd , void *buffer, size_t length, int flags);
//Returns number of bytes received, 0 on EOF, or –1 on error
ssize_t send(int sockfd , const void *buffer, size_t length, int flags);
//Returns number of bytes sent, or –1 on error
```
它们和`read(), write()`的区别只在于第四个`flag`参数，对于读  

| 参数名       | 行为                                             |
|--------------|--------------------------------------------------|
| MSG_DONTWAIT | 非阻塞读写，如果没有数据，发生**EAGAIN**错误     |
| MSG_OOB      | 读超过界限的数据                                 |
| MSG_PEEK     | 读数据时不从buffer中移除                         |
| MSG_WAITALL  | 当可读数据小于需要的长度，阻塞知道读到足够的数据，可以代替`readn`函数 |
|              |                          |

对于写：  
| 参数名       | 行为                                                 |
|--------------|------------------------------------------------------|
| MSG_DONTWAIT | 非阻塞写，如果写不进去（buffer满了），**EAGAIN**错误 |
| MSG_NOSIGNAL | 写的时候屏蔽**SIGPIPE**信号（比如对方关闭连接）      |
| MSG_OOB      |                                                      |

## sendfile()系统调用
正常用循环`write()`写大文件的话比较低效，因为使用了两个系统调用，`read()`将数据从内核空间（`buffer cache`）拷贝到用户空间然后`write()`将数据从用户空间拷贝到内核空间（`socket buffer`）。  

```c
#include <sys/sendfile.h>
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
//Returns number of bytes transferred, or –1 on error
```
`out_fd`是`socket fd`，而`in_fd`必须能用`mmap()`，所以这个函数只能上传文件而不能下载文件。  
### 使用TCP_CORK去优化TCP性能
正常的两次写会发出两个TCP报文段，而开启**TCP_CORK**会将写的数据放到一个单独的TCP段，直到：  
1. 报文段最大长度到了
2. **TPC_SOCK**关闭
3. `socket`关闭
4. 200毫秒的规定时间到了（目的是为了确保写的数据总是会发出去）

诚然，我们可以通过拼接buffer的方式实现**TCP_CORK**类似的功能，但是要想利用**sendfile()**这个函数的话，就需要使用**TCP_CROK**了。


## getsockname(), getpeername()
用于获取对应`sfd`的地址信息和流Socket对应的对方`socket`信息。

## TCP深探
### TCP段格式
HOLD


### 使用shutdown()函数
`shutdown()`的**SHUT_RD**并不是在TCP层面上实现的，不同系统有不同实现，Linux内核只是在用户读的时候返回**EOF**，而实际上另一方依然发送数据的话，在本地`socket`上还是会读到数据的。综上，使用**SHUT_RD**不适合移植。
### TIME_WAIT状态
目的  
1. 保证可靠地关闭连接
2. 保证在网络中的旧重复段失效，使之不能被新的连接所感知

它的`time_out`是两倍的**MSL**
#### 保证可靠地关闭连接
由TCP状态变迁图可知，假设发起主动关闭的一方（client）最后发送的ACK在网络中丢失，由于TCP协议的重传机制，执行被动关闭的一方（server）将会重发其FIN，在该FIN到达client之前，client必须维护这条连接状态，也就说这条TCP连接所对应的资源（client方的local_ip,local_port）不能被立即释放或重新分配，直到另一方重发的FIN达到之后，client重发ACK后，经过2MSL时间周期没有再收到另一方的FIN之后，该TCP连接才能恢复初始的CLOSED状态。如果主动关闭一方不维护这样一个TIME_WAIT状态，那么当被动关闭一方重发的FIN到达时，主动关闭一方的TCP传输层会用RST包响应对方，这会被对方认为是有错误发生，然而这事实上只是正常的关闭连接过程，并非异常。
### 保证在网络中的旧重复段失效，使之不能被新的连接所感知

为说明这个问题，我们先假设TCP协议中不存在TIME_WAIT状态的限制，再假设当前有一条TCP连接：(local_ip, local_port, remote_ip,remote_port)，因某些原因，我们先关闭，接着很快以相同的四元组建立一条新连接。本文前面介绍过，TCP连接由四元组唯一标识，因此，在我们假设的情况中，TCP协议栈是无法区分前后两条TCP连接的不同的，在它看来，这根本就是同一条连接，中间先释放再建立的过程对其来说是“感知”不到的。这样就可能发生这样的情况：前一条TCP连接由local peer发送的数据到达remote peer后，会被该remot peer的TCP传输层当做当前TCP连接的正常数据接收并向上传递至应用层（而事实上，在我们假设的场景下，这些旧数据到达remote peer前，旧连接已断开且一条由相同四元组构成的新TCP连接已建立，因此，这些旧数据是不应该被向上传递至应用层的），从而引起数据错乱进而导致各种无法预知的诡异现象。作为一种可靠的传输协议，TCP必须在协议层面考虑并避免这种情况的发生，这正是TIME_WAIT状态存在的第2个原因。
## Socket选项
```c
#include <sys/socket.h>
int getsockopt(int sockfd, int level, int optname, void *optval,
               socklen_t *optlen);
int setsockopt(int sockfd, int level, int optname, const void *optval,
               socklen_t optlen);
//Both return 0 on success, or –1 on error
```
`socket options`是对应于`file description`的，所以设置了之后，`dup(), fork()`后的都能用。
## SO_REUSEADDR选项
为了解决当关闭连接后，之前连接的端口不能立刻用作新的连接的问题。  
1. `close()`后，在**TIME_WAIT**状态
2. 服务器创建了子进程去处理连接，服务器关闭后，子进程继续处理请求？？？

开启之后我们就可以复用本机端口了。
## 标志和选项（flag, options）的继承性
以下属性在`accept()`后不继承：  
1. 能用`fcntl() F_SETL`修改的标志，如**O_NONBLOCK, OASYNC**
2. 能用`fcntl() F_SETD`修改的标志，如**FD_CLOEXEC**
3. **F_SETOWN, F_SETSIG**标志

## TCP vs UDP
### 什么时候选择UDP?  
1. 一个UDP server可以接受多个客户端的数据报而不需要创建、关闭连接。
2. 对于简单“请求、响应”服务器，UDP效率更高，**TCP = RTT*2 + SPT; UDP = RTT + SPT**，**DNS**就是其中的代表。
3. 可以**广播、多播**
4. 视频流和音频流的传输不需要TCP提供的可靠性，TCP的错误处理会使之产生延时，这是不能接受的。
### UDP想保证可靠怎么办？
自己实现**序列号、确认号、重发、重复检测**的功能，然而如果想更进一步实现**流量控制、拥塞控制**，最好用TCP。

## 高级特性
### Out-of-Band Data
就是提高传输数据的优先级别，在远程登陆的软件`telnet, rlogin, ftp`上很常用，因为要取消之前执行的命令。
1. 使用`send(), recv()`设置**MSG_OOB**标志，`socket`读到后产生**SIGURG**中断（之前得用`fcntl()`设置**F_SETOWN**标志）。

不过并不建议使用这个，可以创建两个连接，一个正常传输，一个传输高优先级的数据。
