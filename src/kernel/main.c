extern void console_init();
extern void gdt_init();
extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();
extern void set_alarm();
extern void memory_map_init();
extern void memory_test();
void kernel_init()
{
    memory_map_init();
    interrupt_init();

    // clock_init();
    // time_init();
    // rtc_init();

    // memory_test();

    asm volatile("sti\n");
    hang();
    return;
}