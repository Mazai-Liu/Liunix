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

        // 保证新进程获得mutex，避免饥饿
        task_yield();
    }

    // 回复之前的中断状态
    set_interrupt_state(intr);
}


void lock_init(lock_t *lock) {
    mutex_init(&lock->mutex);
    lock->repeat = 0;
    lock->holder = NULL;
}

// 尝试持有锁
void lock_acquire(lock_t *lock) {
    task_t *current = running_task();
    if(lock->holder != current) {
        mutex_lock(&lock->mutex);
        lock->holder = current;
        assert(lock->repeat == 0);
        lock->repeat = 1;
    } else {
        lock->repeat++;
    }
}

// 释放锁
void lock_release(lock_t *lock) {
    task_t *current = running_task();
    assert(lock->holder == current);
    lock->repeat--;
    if(lock->repeat == 0) {
        mutex_unlock(&lock->mutex);
        lock->holder = NULL;
        
    }
}
