#include<liunix/syscall.h>

// 0个参数
static inline u32 _syscall0(u32 nr)
{
    u32 ret;
    asm volatile(
        "int $0x80\n"   // 2. 调用0x80中断
        : "=a"(ret)     // 3. 将eax值写入到ret
        : "a"(nr));   // 1.首先将系统调用号放到eax
    return ret;
}

u32 test() {
    return _syscall0(SYS_NR_TEST);
}

void yield() {
    return _syscall0(SYS_NR_YIELD);
}