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

### 为什么不直接传地址？
因为对于不同的`domain`类型，地址的形式是不一样的，而c语言没有重载，所以需要对三种类型对外提供一个公共的接口。
## 流socket

