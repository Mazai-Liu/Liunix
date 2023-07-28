#ifndef LIUNIX_MUTEX_H
#define LIUNIX_MUTEX_H

#include<liunix/types.h>
#include<liunix/task.h>

typedef struct mutex_t
{
    bool value;     // 信号量
    list_t waiters; // 等待列表
} mutex_t;



void mutex_init();
void mutex_lock();
void mutex_unlock();

#endif