global task_switch
task_switch:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov eax, esp
    and eax, pxfffff000  ; current

    mov [eax], esp
    mov eax, [ebp + 8] ;next
    mov esp, [eax]

    pop edi
    pop esi
    pop ebx
    pop ebp

    ret