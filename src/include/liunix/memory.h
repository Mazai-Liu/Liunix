#ifndef LIUNIX_MEMORY_H
#define LIUNIX_MEMORY_H
#include<liunix/types.h>
#define PAGE_SIZE 0x1000  // 4K，每页大小
#define MEMORY_BASE 0x100000 // 可用内存开始的位置

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
    u8 shared : 1;   // 共享内存页，与 CPU 无关
    u8 privat : 1;   // 私有内存页，与 CPU 无关
    u8 readonly : 1; // 只读内存页，与 CPU 无关
    u32 index : 20;  // 页索引
} _packed page_entry_t;

// 得到 cr2 寄存器
u32 get_cr2();

// 得到 cr3 寄存器
u32 get_cr3();

// 设置 cr3 寄存器，参数是页目录的地址
void set_cr3(u32 pde);


#endif