# Liunix
操作系统实现。

根据B站up主[踌躇月光](https://space.bilibili.com/491131440)发布的onix内容来学习完成。

学习、实践的过程比较艰难，会做好笔记以及绘制结构图、流程图等便于自己理解。

期间学到和理解了很多知识，希望自己可以坚持下去。

---

目前实现的模块：

* [x] MBR引导、内核加载
* [x] 内存初始化
* [x] 内存映射和管理
* [x] 进程初步、系统调用初步
* [x] 基础任务、互斥量和锁

---

## 启动

### x86实模式物理内存布局

<img src=".\imgs\README\image-20230628224728936.png" alt="image-20230628224728936" style="zoom:150%;" />

### 启动流程

<img src=".\imgs\README\实模式内存布局.drawio-1688023828911.svg" alt="实模式内存布局.drawio" style="zoom:150%;" />

1. 通电，CPU处于实模式。CS=0xFFFF，IP=0x0000	===>	0xFFFF0。

    > 实模式寻址为： CS << 4 + IP
    >
    > 寄存器16位，而实模式需要访问1M内存（20位），即0x00000~0xFFFFF
    >
    > 0xFFFF0~0xFFFFF为BIOS入口地址
    >
    > 0xF0000~0xFFFFF为系统BIOS地址范围

2. BIOS自检后，将磁盘0磁道0扇区（主引导扇区）读入0x7c00。设置CS=0x7c0，IP=0x0000	===>	0x7c00。

    > 主引导扇区MBR (Master Boot Record)，0x7c00 ~ 0x7DFF
    >
    > MBR 512B，最后两个字节必须是0x55、0xaa

3. 主引导扇区中读入内核加载器loader到0x1000（自指定），跳转到loader执行。

4. loader进行内存检测（ards检测）、加载gdt、进入保护模式、读入内核程序到0x10000，跳转到内核执行。

    > Address Range Descriptor Structure / ARDS 地址范围描述符    20字节
    >
    > Global Descriptor Table / GDT 全局描述符表   8字节

## 内存初始化

内存检测时，保存了ards的数量和位置。

根据ards统计物理内存大小和开始位置（memroy_base），并计算页面数，每页4KB。

<img src=".\imgs\README\内存初始化图.drawio.svg" alt="内存初始化图.drawio" style="zoom:150%;" />

### 内存映射

物理内存开始的一些页（一页管理4k页，即1B映射1页），用来映射物理内存。

映射页即memory_map，他记录了total_pages的被引用的次数。

```cpp
static u32 start_page = 0;   // 可分配物理内存起始位置
static u8 *memory_map;       // 物理内存数组
static u32 memory_map_pages; // 物理内存数组占用的页数
// 物理内存开始的memory_map_pages个页，用来管理物理内存
void memory_map_init()
{
    // 初始化物理内存数组
    memory_map = (u8 *)memory_base;

    // 计算物理内存数组占用的页数。一页管理4k页，即1B管理1页
    memory_map_pages = div_round_up(total_pages, PAGE_SIZE);
    LOGK("Memory map page count %d\n", memory_map_pages);

    // 减去开始的管理页
    free_pages -= memory_map_pages;

    // 清空物理内存数组
    memset((void *)memory_map, 0, memory_map_pages * PAGE_SIZE);

    // 前 1M 的内存位置 以及 映射页已占用的页，已被占用
    start_page = IDX(MEMORY_BASE) + memory_map_pages;
    for (size_t i = 0; i < start_page; i++)
    {
        memory_map[i] = 1;
    }

    LOGK("Total pages %d free pages %d\n", total_pages, free_pages);
}
```

### 页目录和页表

**页目录和页表的每一项都是一个页表项（Page Table Entry，pte)，每个pte大小为4B。**

```c++
typedef struct page_entry_t
{
    u8 present : 1;  // 在内存中
    u8 write : 1;    // 0 只读 1 可读可写
    u8 user : 1;     // 1 所有人 0 超级用户 DPL < 3
    u8 pwt : 1;      // page write through 1 直写模式，0 回写模式
    u8 pcd : 1;      // page cache disable 禁止该页缓冲
    u8 accessed : 1; // 被访问过，用于统计使用频率
    u8 dirty : 1;    // 脏页，表示该页缓冲被写过
    u8 pat : 1;      // page attribute table 页大小 4K/4M
    u8 global : 1;   // 全局，所有进程都用到了，该页不刷新缓冲
    u8 ignored : 3;  // 该安排的都安排了，送给操作系统吧
    u32 index : 20;  // 页索引
} _packed page_entry_t;
```

地址构成：

<img src="./imgs/README/Liunix-neicunyingshe.drawio-1688132466980.svg" alt="Liunix-neicunyingshe.drawio-1688132466980" />

页目录pte：

<img src=".\imgs\README\memory_pde.drawio.svg" alt="memory_pde.drawio" />

页表pte：

<img src=".\imgs\README\memory_pte.drawio.svg" alt="memory_pte.drawio" />

地址映射关系：

<img src=".\imgs\README\Liunix-页目录（二级页表）.drawio-1688132428685.svg" alt="Liunix-页目录（二级页表）.drawio" />

### 内核内存映射

<img src=".\imgs\README\Liunix-内核内存映射.drawio.svg" alt="Liunix-内核内存映射.drawio" />

内核内存8M，需要2页页表。

```cpp
// 内核页目录
#define KERNEL_PAGE_DIR 0x1000
// 内核页表
static u32 KERNEL_PAGE_TABLE[] = {
    0x2000,
    0x3000,
};
```

将最后一个页表指向页目录自己，方便修改。（没搞懂）

### 内存布局

<img src=".\imgs\README\Liunix-内存布局.drawio.svg" alt="Liunix-内存布局.drawio" />

## 内核线程

<img src=".\imgs\README\Liunix-内核进程.drawio.svg" alt="Liunix-内核进程.drawio" />

线程（进程）结构体，task_t。

```c++
// PCB
typedef struct task_t
{
    u32 *stack;                         // 内核栈
    // list_node_t node;                   // 任务阻塞节点
    task_state_t state;                 // 任务状态
    u32 priority;                       // 任务优先级
    int ticks;                          // 剩余时间片
    u32 jiffies;                        // 上次执行时全局时间片
    char name[TASK_NAME_LEN];           // 任务名
    u32 uid;                            // 用户 id
    u32 gid;                            // 用户组 id
    // pid_t pid;                          // 任务 id
    // pid_t ppid;                         // 父任务 id
    // pid_t pgid;                         // 进程组
    // pid_t sid;                          // 进程会话
    // dev_t tty;                          // tty 设备
    u32 pde;                            // 页目录物理地址
    struct bitmap_t *vmap;              // 进程虚拟内存位图
    u32 text;                           // 代码段地址
    u32 data;                           // 数据段地址
    u32 end;                            // 程序结束地址
    u32 brk;                            // 进程堆内存最高地址
    int status;                         // 进程特殊状态
    // pid_t waitpid;                      // 进程等待的 pid
    char *pwd;                          // 进程当前目录
    // struct inode_t *ipwd;               // 进程当前目录 inode program work directory
    // struct inode_t *iroot;              // 进程根目录 inode
    // struct inode_t *iexec;              // 程序文件 inode
    u16 umask;                          // 进程用户权限
    // struct file_t *files[TASK_FILE_NR]; // 进程文件表
    // u32 signal;                         // 进程信号位图
    // u32 blocked;                        // 进程信号屏蔽位图
    // struct timer_t *alarm;              // 闹钟定时器
    // struct timer_t *timer;              // 超时定时器
    // sigaction_t actions[MAXSIG];        // 信号处理函数
    // struct fpu_t *fpu;                  // fpu 指针
    u32 flags;                          // 特殊标记
    u32 magic;                          // 内核魔数，用于检测栈溢出
} task_t;
```

## 系统调用

系统调用是用户进程与内核沟通的方式。可以将 CPU 从用户态转向内核态；

用中断门来实现系统调用，与 linux 兼容，使用 `0x80` 号中断函数；

其中 linux 32 位：

- eax 存储系统调用号

- ebx 存储第一个参数

- ecx 存储第二个参数

- edx 存储第三个参数

- esi 存储第四个参数

- edi 存储第五个参数

- ebp 存储第六个参数

- eax 存储返回值

系统调用的过程

```asm
  mov eax, 系统调用号
  mov ebx, 第一个参数
  mov ecx, 第二个参数
  mov edx, 第三个参数
  int 0x80;
```

## 基础任务

### 空闲进程 idle

用于在所有进程都阻塞的情况下，调度执行，一般进程 id 为 0

主要工作为：

1. 开中断

2. 关闭 CPU，等待外中断

3. yield 调度执行其他程序

### 初始化进程 init

用于执行初始化操作，进入到用户态等工作，进程 id 为 1

## 锁



### 互斥锁

```c++
typedef struct lock_t
{
  struct task_t *holder; // 持有者
  mutex_t mutex;     // 互斥量
  u32 repeat;       // 重入次数
} lock_t;
void lock_init(lock_t *lock);  // 锁初始化
void lock_acquire(lock_t *lock);  // 加锁
void lock_release(lock_t *lock); // 解锁
```

### 自旋锁

自旋锁最多只能被一个可执行线程持有。如果一个执行线程试图获得一个被已经持有（即所谓的争用）的自旋锁，那么该线程就会一直进行忙循环一旋转一等待锁重新可用。要是锁未被争用，请求锁的执行线程便能立刻得到它，继续执行。在任意时间，自旋锁都可以防止多于一个的执行线程同时进入临界区。同一个锁可以用在多个位置，例如，对于给定数据的所有访问都可以得到保护和同步。



使用 CAS (Compare And Swap) 原语实现，IA32 中是 `cmpxchg` 指令，但在多处理机系统中 `cmpxchg` 指令本身不是原子的，还需要加 `lock` 来锁定内存总线实现原子操作。

### 读写锁

有时，锁的用途可以明确地分为读取和写入两个场景。并且绝大多数是读的情况，由于读并不影响数据内容，所以如果直接加锁就会影响性能，那么可以将读和写区别开来，这种形式的锁就是读写锁；

当对某个数据结构的操作可以像这样被划分为读／写 或者 消费者／生产者 两种类别时，类似读／写锁这样的机制就很有帮助了。这种自旋锁为读和写分别提供了不同的锁。一个或多个读任务可以并发地持有读者锁：相反，用于写的锁最多只能被一个写任务持有，而且此时不能有并发的读操作。有时把读／写锁叫做共享排斥锁，或者 并发／排斥锁，因为这种锁以共享（对读者而言）和排斥（对写者而言）的形式获得使用。

但在实现的时候需要注意，有可能会发生读者过多而饿死写着的情况。如果写的情况比较多就不应该使用这种锁。

## 键盘中断

- 0x21 键盘中断向量

| 端口 | 操作类型 | 用途       |
| ---- | -------- | ---------- |
| 0x60 | 读/写    | 数据端口   |
| 0x64 | 读       | 状态寄存器 |
| 0x64 | 写       | 控制寄存器 |
