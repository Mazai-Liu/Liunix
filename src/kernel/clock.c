#include <liunix/io.h>
#include <liunix/interrupt.h>
#include <liunix/assert.h>
#include <liunix/debug.h>
#include <liunix/task.h>

#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000 / HZ)

#define SPEAKER_REG 0x61
#define BEEP_HZ 440
#define BEEP_COUNTER (OSCILLATOR / BEEP_HZ)
#define BEEP_MS 100

// 时间片计数器
u32 volatile jiffies = 0;
u32 jiffy = JIFFY;

bool volatile beeping = 0;

void start_beep()
{
    if (!beeping)
    {
        outb(SPEAKER_REG, inb(SPEAKER_REG) | 0b11);
        // beeping = true;

        // task_sleep(BEEP_MS);

        // outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xfc);
        // beeping = false;
    }
    beeping = jiffies + 5;
}

void stop_beep() {
    if(beeping && jiffies > beeping) {
        outb(SPEAKER_REG, inb(SPEAKER_REG) & 0xfc);
        beeping = false;
    }
}

void clock_handler(int vector)
{
    assert(vector == 0x20);
    send_eoi(vector); // 发送中断处理结束
    stop_beep();

    jiffies++;
    // DEBUGK("clock jiffies %d ...\n", jiffies);

    task_wakeup();

    task_t *task = running_task();
    assert(task->magic == LIUNIX_MAGIC);

    task->jiffies = jiffies;
    task->ticks--;
    if (!task->ticks)
    {
        task->ticks = task->priority;
        schedule();
    }
}

// extern u32 startup_time;

// time_t sys_time()
// {
//     return startup_time + (jiffies * JIFFY) / 1000;
// }

void pit_init()
{
    // 配置计数器 0 时钟
    outb(PIT_CTRL_REG, 0b00110100);
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);

    // 配置计数器 2 蜂鸣器
    outb(PIT_CTRL_REG, 0b10110110);
    outb(PIT_CHAN2_REG, (u8)BEEP_COUNTER);
    outb(PIT_CHAN2_REG, (u8)(BEEP_COUNTER >> 8));
}

void clock_init()
{
    pit_init();
    set_interrupt_handler(IRQ_CLOCK, clock_handler);
    set_interrupt_mask(IRQ_CLOCK, true);
}