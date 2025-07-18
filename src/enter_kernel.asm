[bits 32]
[extern kernel_main]

global _start

_start:
    mov esp, 0x90000
    call kernel_main

.hang:
    hlt
    jmp .hang