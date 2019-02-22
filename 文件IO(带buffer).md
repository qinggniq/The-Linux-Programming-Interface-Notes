# 文件IO（带buffer）
1. I/O系统调用和标准C库提供的I/O函数的区别
2. I/O buffer如何影响性能
3. 如何关闭、打开文件buffer

## 内核I/O的内核缓冲
当使用`read(), write()`系统调用时，并不是直接写到磁盘中，而是简单地将数据从用户空间拷贝到内核空间（我们说这个系统调用并不是和磁盘操作*同步*的），然后如果其它进程要读这个文件的话，就直接从**Buffer Cache**中读了。  
只有当可用内存都没有的时候，内核才会刷新一下，将一些修改过的buffer页写入到磁盘。  
文件的IO时间主要花在磁盘读写上，可以通过实验来得到从用户空间拷贝数据到内核空间的时间。

## 标准I/O （stdio）

```c
#include <stdio.h>
int setvbuf(FILE *stream, char *buf, int mode, size_t size);
Returns 0 on success, or nonzero on error

void setbuf(FILE *stream. char *buf);
```

### buf
- `buf`非空，使用`buf`作为`stream`的buffer，所以应该分配在堆中，或者静态区域。
- `buf`为空，库自动分配
### mode
| name   | meaning      |
| -----  | ----         |
| _IONBF | 不设置缓冲区 |
| _IOLBF | 行缓冲       |
| _IOFBF | 全缓冲       |

### setbuf()
和`setvbuf(fp, buf, (buf != NULL)? _IOFBF:_IONBF, BUFSIZ);`一样

### fflush()
```c
#include <stdio.h>
int fflush(FILE *stream);
Returns 0 on success, EOF on error
```
1. `stream`为空，刷新所有stdio buffer。  
2. 对于写流，`fflush()`将调用`write()`系统调用将缓冲区的内容拷贝到内核缓冲区。  
3. 对于读流，`fflush()`将丢弃掉stdio输入缓冲里面的内容。  
4. 文件关闭自动flush。


## 控制内核Buffer
### 目的
有时我们需要强制将内核buffer的内容写到磁盘，如错误日志在电脑断电的时候。
### SUSv3的（数据同步IO，文件同步IO）
#### 文件元数据
即描述文件的数据，如文件大小、修改时间、block节点指针。
#### 数据同步IO
- 读时，如果对这个文件有写的操作，则先把文件和元数据写入文件
- 写时，把数据和元数据都写进去
#### 文件同步IO
和数据同步IO的区别在于文件同步IO把**元数据**全部写进去，而数据同步IO只写**部分元数据**用来*retrival*
#### 系统调用
```c
#include <unistd.h>
int fsync(int fd);
Returns 0 on success, or –1 on error
```

```c
#include <unistd.h>
int fdatasync(int fd);
Returns 0 on success, or –1 on error
``
两者的区别在于一个是数据同步IO一个是文件同步IO，当某些**元数据**不变时，数据同步IO不会更新它而文件同步IO会，所以数据同步IO相比于文件同步IO性能更高。

```c
#include <unistd.h>
void sync(void);
```
刷新所有buffer

#### O_SYNC
`open`系统调用`mode`加O_SYNC会使这个文件描述符的所有写操作同步。

### 给内核读写提建议
```c
#define _XOPEN_SOURCE 600
#include <fcntl.h>
int posix_fadvise(int fd, off_t offset, off_t len, int advice);
Returns 0 on success, or a positive error number on error
```
| 参数   | 含义                                                |
|--------|-----------------------------------------------------|
| fd     | 我们想建议的文件描述符                              |
| offset | 区域的开始偏移量                                    |
| len    | 区域长度，长度为0表示从开始到文件尾都是感兴趣的区域 |
| advice | 希望内核读取的方式                                                    |

关于advice具体内容和它导致内核相应的行为参见书的245页。

### 直接IO
1. 一般性能低下，主要应用于特殊场景，如数据库有自己的cache不需要内核缓冲。
2. O_DIRCT标志。
3. 需要对齐







