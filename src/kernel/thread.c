#include<liunix/debug.h>
#include<liunix/interrupt.h>
#include<liunix/syscall.h>
#include<liunix/mutex.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void idle_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        // LOGK("idle task...%d\n", counter++);
        asm volatile(
            "sti\n" // 开中断
            "hlt\n" // 关闭cpu，进入暂停，等待外中断到来
        );
        yield();
    }
}
mutex_t *mutex;
void init_thread() {
    mutex_init(&mutex);
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        mutex_lock(&mutex);
        LOGK("init_task...%d\n", counter++);
        mutex_unlock(&mutex);
        // sleep(500);
    }
}

void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        mutex_lock(&mutex);
        LOGK("test_task...%d\n", counter++);
        mutex_unlock(&mutex);
        // sleep(803);
    }
}