[bits 32]

section .text

global inb  ; 将inb导出，用于链接
inb:
    push	ebp
	mov	ebp, esp 

    xor eax, eax
    mov edx, [ebp + 8] ; port
    in al, dx   ; 将端口号dx的8bit输入到al
    jmp $ + 2  ; 一点延迟
    jmp $ + 2
    jmp $ + 2

    leave
	ret
global inw  ; 将inw导出，用于链接
inw:
    push	ebp
	mov	ebp, esp 

    xor eax, eax
    mov edx, [ebp + 8] ; port
    in ax, dx   ; 将端口号dx的8bit输入到al
    jmp $ + 2  ; 一点延迟
    jmp $ + 2
    jmp $ + 2

    leave
	ret

global outb  ; 将outb导出，用于链接
outb:
    push	ebp
	mov	ebp, esp 

    mov edx, [ebp + 8] ; port
    mov eax, [ebp + 12] ; value
    out dx, al   ; 将al的8bit输入到端口号dx


    jmp $ + 2  ; 一点延迟
    jmp $ + 2
    jmp $ + 2

    leave
	ret

global outw  ; 将outw导出，用于链接
outw:
    push	ebp
	mov	ebp, esp 

    mov edx, [ebp + 8] ; port
    mov eax, [ebp + 12] ; value
    out dx, ax   ; 将al的16bit输入到端口号dx
    jmp $ + 2  ; 一点延迟
    jmp $ + 2
    jmp $ + 2

    leave
	ret