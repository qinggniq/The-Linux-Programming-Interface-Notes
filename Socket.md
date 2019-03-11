# Socket
## 我想学到的东西
1. 常用的socket的函数
2. 错误处理方式
3. 一些与协议相关的流程细节
## socket概述
```c
socket(domain, type, protocol);
```
### domain
1. 可以辨识一个socket
2. 通信的范围
| 参数值   | 含义                       | 地址格式     |
| ---      | ---                        | ----         |
| AF_UNIX  | 单机通信                   | pathname     |
| AF_INET  | 使用IPV4协议进行的多机通信 | 32ip:16port  |
| AF_INET6 | 使用IPv6协议进行的多机通信 | 128ip:16port |

### type
主要区分是面向**数据流**还是**数据报**的  
数据流  
1. 可靠
2. 双向
3. 字节流
### ps
有关`socket()`的系统调用默认是阻塞的，所以如果没有数据的话会挂起，可以通过`fcntl()`来设置为非阻塞。

## 创建一个socket
```c
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
Returns file descriptor on success, or –1 on error
```
`domain`和`type`如上，`protocol`一般为0，其他的不考虑。
### ps
`type`除了什么TCP、UDP参数外，还可以和一些读写标志“或”一下，比如**SOCK_CLOEXEC**程序结束，关闭`socket`，**SOCK_NONBLOCK**设置对`socket`读写不阻塞。
## 绑定socket
```c
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
//Returns 0 on success, or –1 on error
```
如果不`bind()`直接`listen()`的话，系统使用本机ip和默认端口。

## 通用socket地址结构体
### 结构体
```c
struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};

```
permission
### 为什么不直接传地址？
因为对于不同的`domain`类型，地址的形式是不一样的，而c语言没有重载，所以需要对三种类型对外提供一个公共的接口。
## 流socket
`流socket`的连接步骤如图：  
TODO
### listen()函数
```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
Returns 0 on success, or –1 on error
```

`sockfd`参数不能是已经`connect()`过，或者`accpet()`返回的socket
#### backlog参数
当用户在服务器调用`accpet()`之前`connect()`则这个`connect`叫做“挂起连接”。`backlog`设定了服务器内核记录“挂起连接”的最大数目，在最大数目后面的`connect`被阻塞。

### accept()函数
```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
//Returns file descriptor on success, or –1 on error
```
`sockfd`是绑定`ip`和`port`的描述符，`addr, addrlen`存储了客户端的地址信息，如果不感兴趣把它们设置为`NULL, 0`，`aadrlen`既是输入又是输出，作为输入应该为`sockaddr`结构体长度，作为输出应该为实际写入结构体的长度。
#### ps
`accept4()`提供了新的、设置返回的文件描述符标志的参数，`SOCK_CLOEXEC, SOCK_NONBLOCK`。

### connect()函数

```c
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
//Returns 0 on success, or –1 on error
```
`sockfd`是`sockect()`返回的文件描述符，后面的参数是要连接的`socket`的地址信息。
### 流Socket的IO
1. 用`read(), write()`系统调用进行IO
2. 当一方用`close()`关闭连接后，另一方读会读到**EOF**标志，另一方写会收到**SIGPIPE**信号，和**EPIPE**错误。处理方法在44.2节。
### close()函数
如果多个文件描述符指向同一个`socket`，则`close()`会关闭所有指向它的文件描述符。

## 数据报Socket
1. `socket()`系统调用相当于设置邮箱。
2. `bind()`将自己绑定到对应的地址。
3. `sendto()`发报文，`revfrom()`收报文。
4. `close()`关闭`socket`。  
图56-4，TODO
### sendto(), recvfrom()函数
```c
#include <sys/socket.h>
ssize_t recvfrom(int sockfd, void *buffer, size_t length, int flags,
struct sockaddr *src_addr, socklen_t *addrlen);
//Returns number of bytes received, 0 on EOF, or –1 on error
ssize_t sendto(int sockfd, const void *buffer, size_t length, int flags,
const struct sockaddr * dest_addr, socklen_t addrlen);
//Returns number of bytes sent, or –1 on error
```

`recvfrom()`函数的`length`长度小于实际接受到的信息的长度，信息会被截断。

### connect函数
如果使用了`connect()`内核就会记录`socket`相应的地址信息，从而可以使用`write()`系统调用来发数据报。

