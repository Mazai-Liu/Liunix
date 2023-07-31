#include<liunix/interrupt.h>
#include<liunix/io.h>
#include<liunix/assert.h>
#include<liunix/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CTRL_PORT 0x64

void keyboard_handler(int vector) {
    assert(vector == 0x21);
    send_eoi(vector);  // 发送中断处理完成信号
    u16 scancode = inb(KEYBOARD_DATA_PORT);   // 从键盘获取按键扫描码，不读中断不会继续发生
    LOGK("keyboard input: 0x%x\n", scancode);
}

void keyboard_init() {
    set_interrupt_handler(IRQ_KEYBOARD, keyboard_handler);
    set_interrupt_mask(IRQ_KEYBOARD, true);
}