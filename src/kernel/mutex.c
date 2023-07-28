#include<liunix/mutex.h>
#include<liunix/list.h>
#include<liunix/interrupt.h>
#include<liunix/task.h>
#include<liunix/assert.h>

void mutex_init(mutex_t *mutex) {
    mutex->value = false;
    list_init(&mutex->waiters);
}

// p操作
void mutex_lock(mutex_t *mutex) {
    // // p操作
    // mutex->value--;
    // if(mutex->value < 0)
    //     task_block(current, mutex->waiters, TASK_BLOCKED);
    
    // 关闭中断确保原子操作
    bool intr = interrupt_disable();

    task_t *current = running_task();


    while(mutex->value == true) {
        task_block(current, &mutex->waiters, TASK_BLOCKED);
    }

    assert(mutex->value == false);
    mutex->value++;
    assert(mutex->value == true);

    // 回复之前的中断状态
    set_interrupt_state(intr);
}


// v操作
void mutex_unlock(mutex_t *mutex) {
    // // v操作
    // mutex->value++;
    // if(mutex->value >= 0)
    //     task_wakeup();

    // 关闭中断确保原子操作
    bool intr = interrupt_disable();

    task_t *current = running_task();

    assert(mutex->value == true);
    mutex->value--;
    assert(mutex->value == false);
    

    if(!list_empty(&mutex->waiters)) {
        task_t *task = element_entry(task_t, node, mutex->waiters.tail.prev);
        assert(task->magic == LIUNIX_MAGIC);
        task_unblock(task);

        // 保证新进程获得mutex，必然饥饿
        task_yield();
    }

    // 回复之前的中断状态
    set_interrupt_state(intr);
}