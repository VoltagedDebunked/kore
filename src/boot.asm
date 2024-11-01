[BITS 64]

extern kernel_main

section .text
global _start
_start:
    cli
    ; Set up the stack
    mov rsp, 0x7FFFFFFFE000

    ; Call the kernel main function (unlikely to fucking work. This is garbage code.)
    call kernel_main

    ; If kernel_main returns, execute hang and loop.
    ; As you can see this jumps to itself, meaning it
    ; will create an infinite loop, allowing the kernel
    ; to run infinitely without potentially crashing.
hang:
    hlt
    jmp hang
