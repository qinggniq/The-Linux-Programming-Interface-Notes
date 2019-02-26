# Socket域
## Unix域（单机）struct sockaddr_un
```c
struct sockaddr_un {
    sa_family_t sun_family;
    char sun_path[108];
};
```
一个`pathname`只能对应一个`socket`，反之亦然。在实际应用中`pathname`对应的文件权限需要合理的安全设置。

## Unix域下的流Socket
和正常的`Client, Server`差不多，但是多了一步是需要在`bind()`前`remove()`对应文件的`socket`

## Unix域下的报文段Socket
如上
## Unix域的权限
不管是流Socket`connect()`还是报文段Socket`sendto()`都需要文件的写权限。并且需要文件目录的执行权限。

## sockpair()
简化了两个`socketfd`连接的过程，只能在**Unix域**中使用。

## Linux抽象命名空间

1. 不必关系命名冲突
2. 不必`unlink`文件，当`socket`使用完毕后
3. 
