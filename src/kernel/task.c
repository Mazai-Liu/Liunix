#include <liunix/task.h>
#include <liunix/printk.h>
#include <liunix/debug.h>
#include <liunix/memory.h>
#include <liunix/assert.h>
#include <liunix/interrupt.h>
#include <liunix/string.h>
#include <liunix/bitmap.h>
#include <liunix/types.h>
#include <liunix/syscall.h>
#include <liunix/list.h>

extern bitmap_t kernel_map;
extern void task_switch(task_t *next);

#define TASK_NR 64

extern u32 volatile jiffies;
extern u32 jiffy;

static task_t *task_table[TASK_NR];     // 任务表
static list_t block_list;               // 任务默认阻塞链表
static list_t sleep_list;               // 任务睡眠链表
static task_t *idle_task;               // 空闲任务

static task_t *get_free_task() {
    for(size_t i = 0; i < 64; i ++) {
        if(task_table[i] == NULL) {
            task_table[i] = (task_t *)alloc_kpage(1);
            return task_table[i];
        }
    }
}

// 从任务数组中查找某种状态的任务，自己除外
static task_t *task_search(task_state_t state)
{
    assert(!get_interrupt_state());
    task_t *task = NULL;
    task_t *current = running_task();

    for (size_t i = 0; i < TASK_NR; i++)
    {
        task_t *ptr = task_table[i];
        if (ptr == NULL)
            continue;

        if (ptr->state != state)
            continue;
        if (current == ptr)
            continue;
        if (task == NULL || task->ticks < ptr->ticks || ptr->jiffies < task->jiffies)
            task = ptr;
    }

    if (task == NULL && state == TASK_READY)
    {
        task = idle_task;
    }

    return task;
}

void task_block(task_t *task, list_t *blist, task_state_t state) {
    assert(!get_interrupt_state());
    assert(task->node.next == NULL);    // 未加入到别的链表中
    assert(task->node.prev == NULL);

    if(blist == NULL) {
        blist = &block_list;
    }

    list_push(blist, &task->node);

    task->state = state;

    task_t *current = running_task();
    if(current == task) {
        schedule();
    }
}

void task_unblock(task_t *task) {
    assert(!get_interrupt_state());
    list_remove(&task->node);

    task->state = TASK_READY;
}

void task_sleep(u32 ms) {
    assert(!get_interrupt_state()); // 不可中断

    u32 ticks = ms / jiffy;
    ticks = ticks > 0 ? ticks : 1;

    // 记录目标全局时间片，咋子那个时刻需要唤醒任务
    task_t *current = running_task();
    current->ticks = jiffies + ticks;

    //从睡眠链表找到第一个比当前任务唤醒时间点更晚的任务，进行插入排序
    list_t *list = &sleep_list;
    list_node_t *anchor = &list->tail ;
    for (list_node_t *ptr = list->head.next; ptr != &list->tail; ptr = ptr->next)
    {
        task_t *task = element_entry(task_t, node, ptr);
        if (task->ticks > current->ticks )
        {
            anchor = ptr ;
            break;
        }
    }

    assert(current->node.next == NULL);    // 未加入到别的链表中
    assert(current->node.prev == NULL);

    list_insert_before(anchor, &current->node);
    current->state = TASK_SLEEPING;
    schedule();
}

void task_wakeup() {
    assert(!get_interrupt_state()); //不可中断
    //从睡眠链表中找到ticks小于等于jiffies的任务，恢复执行
    list_t *list = &sleep_list;
    for(list_node_t *ptr = list->head.next; ptr != &list->tail;)
    {
        task_t *task = element_entry(task_t, node, ptr);
        if (task->ticks > jiffies)
            break;
        
        // unblock会将指针清空
        ptr = ptr->next;
        
        task->ticks=0;
        task_unblock(task);
    }

}


task_t *running_task()
{
    // PCB放到当前任务分配的内存的开头
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

void schedule()
{
    assert(!get_interrupt_state()); // 不可中断

    task_t *current = running_task();
    task_t *next = task_search(TASK_READY);

    assert(next != NULL);
    assert(next->magic == LIUNIX_MAGIC);

    if (current->state == TASK_RUNNING){
        current->state = TASK_READY;
    }

    if (!current->ticks){
        current->ticks = current->priority;
    }

    next->state = TASK_RUNNING;
    if (next == current)
        return;

    // fpu_disable(current); // 当前进程禁用 FPU
    // task_activate(next);  // 激活下一进程
    task_switch(next);    // 调度到下一进程
}

task_t *task_create(target_t target, const char *name, u32 priority, u32 uid)
{
    task_t *task = get_free_task();
    memset(task, 0, PAGE_SIZE);

    u32 stack = (u32)task + PAGE_SIZE;

    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target;

    strcpy((char *)task->name, name);

    task->stack = (u32 *)stack;
    task->priority = priority;
    task->ticks = task->priority;
    task->jiffies = 0;
    task->state = TASK_READY;
    // task->uid = uid;
    // task->gid = 0; // TODO: group
    // task->pgid = 0;
    // task->sid = 0;
    task->vmap = &kernel_map;
    task->pde = KERNEL_PAGE_DIR; // page directory entry
    // task->brk = USER_EXEC_ADDR;
    // task->text = USER_EXEC_ADDR;
    // task->data = USER_EXEC_ADDR;
    // task->end = USER_EXEC_ADDR;
    // task->iexec = NULL;
    // task->iroot = task->ipwd = get_root_inode();
    // task->iroot->count += 2;

    // task->pwd = (void *)alloc_kpage(1);
    // strcpy(task->pwd, "/");

    // task->umask = 0022; // 对应 0755

    // task->files[STDIN_FILENO] = &file_table[STDIN_FILENO];
    // task->files[STDOUT_FILENO] = &file_table[STDOUT_FILENO];
    // task->files[STDERR_FILENO] = &file_table[STDERR_FILENO];
    // task->files[STDIN_FILENO]->count++;
    // task->files[STDOUT_FILENO]->count++;
    // task->files[STDERR_FILENO]->count++;

    // // 初始化信号
    // task->signal = 0;
    // task->blocked = 0;
    // for (size_t i = 0; i < MAXSIG; i++)
    // {
    //     sigaction_t *action = &task->actions[i];
    //     action->flags = 0;
    //     action->mask = 0;
    //     action->handler = SIG_DFL;
    //     action->restorer = NULL;
    // }

    // task->timer = NULL;
    // task->alarm = NULL;

    task->magic = LIUNIX_MAGIC;

    return task;
}

void task_yield() {
    schedule();
}

static void task_setup()
{
    task_t *task = running_task();
    task->magic = LIUNIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}


extern idle_thread();
extern init_thread();
extern test_thread();

void task_init() {
    list_init(&block_list);
    list_init(&sleep_list);

    task_setup();

    idle_task = task_create(idle_thread, "idle", 1, KERNEL_USER);
    task_create(init_thread, "init", 5, NORMAL_USER);
    task_create(test_thread, "test", 5, NORMAL_USER);
}
