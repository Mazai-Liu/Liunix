[bits 32]

extern kernel_init
extern memory_init
extern console_init
extern gdt_init

global _start
_start:
    push ebx    ; ards_count
    push eax    ; magic

    call console_init   ; 初始化控制台
    call gdt_init       ; 初始化全局描述符
    call memory_init    ; 初始化内存
    call kernel_init    ; 初始化内核

    ; int 0x80
    jmp $