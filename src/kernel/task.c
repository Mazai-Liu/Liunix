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

extern bitmap_t kernel_map;
extern void task_switch(task_t *next);

#define TASK_NR 64

task_t *task_table[TASK_NR]; // 任务表,所有任务

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

    // if (task == NULL && state == TASK_READY)
    // {
    //     task = idle_task;
    // }

    return task;
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

u32 thread_a() {
    set_interrupt_state(true);
    while(true) {
        printk("a");
        yield();
    }
}
u32 thread_b() {
    set_interrupt_state(true);
    while(true) {
        printk("b");
        yield();
    }
}
u32 thread_c() {
    set_interrupt_state(true);
    while(true) {
        printk("c");
        yield();
    }
}

void task_init() {
    task_setup();
    task_create(thread_a, "a", 5, KERNEL_USER);
    task_create(thread_b, "b", 5, KERNEL_USER);
    task_create(thread_c, "c", 5, KERNEL_USER);
}
