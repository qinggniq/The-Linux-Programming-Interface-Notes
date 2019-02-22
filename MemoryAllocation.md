# 内存分配
## 在堆上分配内存
### 调整堆顶(program break)brk(), sbrk()
调整堆顶就是把program break值改一下即可, 但实际**物理页**的分配是待访问后才分配
```c
#include <unistd.h>
int brk(void *end_data_segment);
    //Returns 0 on success, or –1 on error
void *sbrk(intptr_t increment);
//Returns previous program break on success, or (void *) –1 on error
```
`brk()`系统调用是将`program break`改成`end_data_segment`, 一般是下一页的界限, 只能设置为比堆底大的地址,否则产生**SIGEGV**异常
堆顶的最大值和进程资源限制有关
### malloc族库函数
相比于`brk(), sbrk()`它有以下优点
1. C标准的一部分
2. 多线程程序使用方便
3. 提供了一个简单的接口
4. 可以让我们很方便的free内存

```c
#include <stdlib.hma>
void *malloc(size_t size);
//Returns pointer to allocated memory on success, or NULL on error
```

```c
#include <stdlib.h>
void free(void *ptr);
```
free掉不是由malloc返回的指针, 则行为未定义  
free并不改变堆顶(program break), 而是将内存放到空闲链表中  
原因
1. 被释放的内存一般在堆中间
2. 要调整堆顶(program break)需要系统调用, 开销不小
3. 一般减小堆顶不会帮助分配大内存

只有将堆free的足够大时(一般128kb), program break才会改变  
### 显式free掉内存
1. 使程序易读
2. 使用debug查找内存泄漏时, 会很难发现真正的内存泄漏

### malloc, free的实现
#### malloc
1. 扫描被`free()`释放的内存块，找到大于等于申请大小的内存块(首次适配，最佳适配。。。)
2. 否则（找不到合适的内存块）使用`sbrk()`去分配更多的内存。为了减小`sbrk()`的调用次数，`sbrk()`会向内核申请更大的内存备用。
#### free
1. `free()`使用返回指针的前一个字节去存储`malloc()`申请的大小
2. 当`free()`将内存放到空闲列表时，将第一个第二次字节存储`pre, next`指针
#### 注意事项
- 不要修改范围外的空间
- 不要`free()`两次
- 不要`free()`不是用`malloc`函数族申请的内存指针
- 在长循环里要注意实时释放掉不用的空间
### malloc调试
- `mtrace(), muntrace()`可以打开、关闭内核对内存分配的记录。使用**MALLOC_TRACE**环境变量，指定记录写在那个文件。
- `mcheck(), mprobe()`实时的检查内存的分配，需要链接*-lmcheck*
- **MALLOC_CHECK_**环境变量，不需要链接，设置为整数代表程序对发生内存错误的不同行为
- 看**glibc**手册
### 其它在堆上分配内存的函数
#### calloc() realloc()
```c
#include <stdlib.h>
void *calloc(size_t numitems, size_t size);
Returns pointer to allocated memory on success, or NULL on error
```
和`malloc()`差不多，多了个对分配内存的初始化（置零）操作

```c
#include <stdlib.h>
void *realloc(void *ptr, size_t size);
Returns pointer to allocated memory on success, or NULL on error
```
在`malloc`族函数之前分配的内存上重新分配大小  
```c
realloc(prt, 0);
//等价于
prt = malloc(0);
free(ptr);

realloc(NULL, size);
//等价于
malloc(size);
```
1. 如果在空闲链表上找到合适的，就直接扩展
2. 如果原有的内存块在最后面增长*program break*
3. 如果在中间并且找不到合适的，就找一个够大的，再复制过去

`realloc()`后，指向之前空间的指针失效
### memalign()自定义空间起始位置强制对齐
## 在栈上分配内存
```c
#include <alloca.h>
void *alloca(size_t size);
Returns pointer to allocated block of memory
```
不能在程序参数中调用`alloc()`。  
与`malloc()`比较  
1. 更快，不需要什么空闲链表，查找适合空间什么的
2. 自动释放

