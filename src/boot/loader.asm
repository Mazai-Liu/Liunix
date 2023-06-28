[org 0x1000]

dw 0x55aa  ; 魔数

mov si, loading
call print

detect_memory:  ; 内存检测
    xor ebx, ebx

    ; 结构体缓存位置
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer

    mov edx, 0x534d4150
    .next
        mov eax, 0xe820 ; 子功能号
        mov ecx, 20 ; ards结构大小 20B
        int 0x15

        ; 如果CF置位，表示出错
        jc error

        add di, cx
        inc word [ards_count]

        cmp ebx, 0
        jnz .next

        mov si, detect_success
        call print

        jmp prepare_protected_mode

prepare_protected_mode:  ; 启动保护模式
    cli ; 关闭中断

    ; 打开A20线
    in al, 0x92
    or al, 0b0000_0010
    out 0x92, al

    ; 加载gdt
    lgdt [gdt_ptr]

    ; 启动保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 用跳转来刷新缓存, 启用保护模式
    jmp dword code_selector:protected_mode

print:
    mov ah, 0x0e
    .next:
        mov al, [si]
        cmp al, 0
        jz .done
        int 0x10
        inc si
        jmp .next
    .done:
        ret

loading:
    db "Loading Liunix......", 10, 13, 0 ; \n \r  回车回到行首
detect_success:
    db "Detect Memory Success...", 10, 13, 0 ; \n \r  回车回到行首
error:
    mov si, .msg
    call print
    hlt  ; 让CPU停止
    jmp $
    .msg db "Error!!!"


[bits 32]
protected_mode:
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax ; 初始化段寄存器

    mov esp, 0x10000 ; 修改栈顶

    mov edi, 0x10000  ; 读取的目标内存
    mov ecx, 10  ; 起始扇区
    mov bl, 200  ; 读写扇区数量
    call read_disk   ; 读入内核
    jmp code_selector:0x10000  ; 跳转到内核执行
    ud2  ; 表示出错
jmp $

read_disk:
    mov dx, 0x1f2 ; 扇区数量端口地址
    mov al, bl
    out dx, al

    inc dx  ; 0x1f3  前8位
    mov al, cl
    out dx, al

    inc dx  ;0x1f4  中八位
    shr ecx, 8
    mov al, cl
    out dx, al

    inc dx  ;0x1f5  高八位
    shr ecx, 8
    mov al, cl
    out dx, al

    inc dx  ;0x1f6 
    shr ecx, 8
    and cl, 0b0000_1111 ; 保留24~27位

    mov al, 0b1110_0000 ; 4~7位为0111【主盘，LBA模式】
    or al, cl     ; 组合为0x1f6
    out dx, al   ;  写入0x1f6

    inc dx ; 0x1f7
    mov al, 0x20  ; 读硬盘
    out dx, al

    xor ecx, ecx
    mov cl, bl  ; 读写扇区数量
    .read
        push cx
        call .waits ; 等待数据准备完毕
        call .reads ; 读取一个扇区
        pop cx
        loop .read
    ret

    .waits  
        mov dx, 0x1f7
        .check
            in al, dx
            jmp $ + 2  ;  加点延迟
            jmp $ + 2
            jmp $ + 2

            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .check
        ret

    .reads  
        mov dx, 0x1f0
        mov cx, 256  ; 一个扇区256个字word
        .readw:
            in ax, dx
            jmp $ + 2  ;  加点延迟
            jmp $ + 2
            jmp $ + 2
            mov [edi], ax
            add edi, 2
            loop .readw
        ret


code_selector equ (1 << 3)
data_selector equ (2 << 3)

memory_base equ 0  ; 内存开始的位置
; 内存界限 4G / 4K - 1
memory_limit equ (1024 * 1024 * 1024) * 4 / (4 * 1024) - 1

gdt_ptr:
    dw (gdt_end - gdt_base) - 1
    dd gdt_base

gdt_base:
    dd 0, 0 ; NULL 描述符
gdt_code:
    dw memory_limit & 0xffff ; 段界限0~15位
    dw memory_base & 0xffff ; 基地址0~15位
    db (memory_base >> 16) & 0xff ;基地址16~23位 
    db 0b1_00_1_1010 ; 存在，dpl 0， 代码段，代码、非依从、可读未被访问过
    ; 4k 32位 不是64位  段界限16~19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xff
    db (memory_limit >> 24) & 0xff  ;// 基地址 24~ 31位
gdt_data:
    dw memory_limit & 0xffff ; 段界限0~15位
    dw memory_base & 0xffff ; 基地址0~15位
    db (memory_base >> 16) & 0xff ;基地址16~23位 
    db 0b1_00_1_0010 ; 存在，dpl 0，数据段，向上，可写，未被访问过
    ; 4k 32位 不是64位  段界限16~19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xff
    db (memory_limit >> 24) & 0xff  ;// 基地址 24~ 31位
gdt_end:

ards_count:
    dw 0
ards_buffer: