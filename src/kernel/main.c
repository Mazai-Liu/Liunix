extern void console_init();
extern void gdt_init();
extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();
extern void set_alarm();
extern void memory_map_init();
extern void memory_test();
extern void mapping_init();
extern void set_interrupt_state();
extern void task_init();
extern void syscall_init();


void kernel_init()
{
    memory_map_init();
    mapping_init();
    interrupt_init();
    clock_init();
    // time_init();
    // rtc_init();

    task_init();
    syscall_init();
    
    set_interrupt_state(1);
    return;
}