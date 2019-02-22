# 文件IO

## 文件IO模型
|函数原型|函数作用|
|---|---|
|int open(str* pathname, flag, mode)|打开文件|
|size_t read(fd, buffer, size)|读取文件|
|size_t write(fd, buffer, size)|写文件|
|void close(fd)|关闭文件|
|lseek(fd, where, offset)|调整当前读取的位置|
## 函数解释

## 原子性
### 竞争条件
#### 检查并创建文件
O_CREAT|O_EXCL
#### 调整读写指针并向文件追加数据
O_APPEND

## 文件描述符操作函数
int fcntl(int fd, int cmd, ...);
 O_APPEND, O_NONBLOCK, O_NOATIME, O_ASYNC, and
O_DIRECT
可以修改以上标志
Get Flag

fcntl(fd, F_GETFL)

Set Flag

fcntl(fd, F_SETFL, flags)
## 文件描述符与打开的文件
1. 进程文件描述符(descriptor)表
2. 系统的打开的文件描述符(description)表
3. 文件系统的i-node表

### 文件描述符表
存在
1. 文件标志集合
2. 到文件描述符(description)的引用
### 系统级别的文件表
1. 当前文件的偏移量
2. 打开文件时设置的状态标志
3. 文件访问模式
4. 与信号驱动IO相关的设置
5. i-node对象的引用
### i-node对象表
1. 文件类型
2. Lock链表指针
3. 文件的各种属性（时间大小）

[](一张图Figure5-2)
dup dup2 => 两个文件描述符(descriptor)指向统一个文件描述符(description
)

open twice => 两个文件描述符(description)指向统一个i-node节点



ssize_t pread(int fd, void *buf, size_t count, off_t offset);

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
## 5.9 非阻塞IO

### 目的
1. 文件不能立即被打开，返回错误（管道）
2. 如果IO操作不能立马完成，要么只读取/写入部分数据、要么EAGIN、EWOULDBLOCK错误

### 注意事项
非阻塞IO可以用到设备、（伪）终端、管道、队列、socket中，而当用于**socket**时，我们需要使用`fcntl`的**F_SETFL**去设置非阻塞。
