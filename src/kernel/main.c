extern void console_init();
extern void gdt_init();
extern void interrupt_init();
extern void clock_init();
extern void hang();
extern void time_init();
extern void set_alarm();

void kernel_init()
{
    console_init();
    gdt_init();
    interrupt_init();

    // clock_init();
    time_init();
    rtc_init();

    set_alarm();
    asm volatile("sti\n");
    hang();

    return;
}