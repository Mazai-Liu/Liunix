[org 0x7c00]

; 设置屏幕为文本模式，清除屏幕
mov ax, 3
int 0x10

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

mov si, booting
call print

mov edi, 0x1000  ; 读取的目标内存
mov ecx, 2  ; 起始扇区
mov bl, 4  ; 读写扇区数量

call read_disk  ; 读入loader

cmp word [0x1000], 0x55aa
jnz error

jmp 0:0x1002

; 阻塞
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

print:
    mov ah, 0x0e
    .next:
        mov al, [si]
        cmp al, 0
        jz .done
        int 0x10
        inc si
        jmp .next
    .done
        ret

booting:
    db "Booting Liunix......", 10, 13, 0 ; \n \r  回车回到行首

error:
    mov si, .msg
    call print
    hlt  ; 让CPU停止
    jmp $
    .msg db "Booting Error!!!"

; 填充0
times 510 - ($ - $$) db 0

; 主引导扇区最后两个字节必须是0x55 0xaa
db 0x55, 0xaa