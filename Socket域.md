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
