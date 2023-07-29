#ifndef LIUNIX_MUTEX_H
#define LIUNIX_MUTEX_H

#include<liunix/types.h>
#include<liunix/task.h>

typedef struct mutex_t
{
    bool value;     // 信号量
    list_t waiters; // 等待列表
} mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

typedef struct lock_t
{
    task_t *holder; // 持有者
    mutex_t mutex; // 互斥量
    u32 repeat;   // 重入次数
} lock_t;

void lock_acquire(lock_t *lock);
void lock_release(lock_t *lock);
void lock_init(lock_t *lock);


#endif