[bits 32]

extern exit
test:
    ret

global main
main:
    call test
    call exit