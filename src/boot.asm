[BITS 64]

extern kernel_main

section .text
global _start
_start:
    cli
    ; Set up the stack
    mov rsp, 0x7FFFFFFFE000

    ; Call the kernel main function
    call kernel_main

    ; If kernel_main returns, hang
hang:
    hlt
    jmp hang
