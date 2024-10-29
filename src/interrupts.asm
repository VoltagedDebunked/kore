[BITS 64]

extern outb
extern inb_asm ; Because assembly already has inb, thats why its inb_asm, no shit...
extern memcpy
global timer_handler
global keyboard_handler
global disk_handler

section .bss
buffer: resb 512

section .text

timer_handler:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov rdi, 0x20        ; First argument: port number
    mov rsi, 0x20        ; Second argument: data
    call outb            ; Call the outb function

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq

keyboard_handler:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, 0x20        ; First argument: port number
    mov rsi, 0x20        ; Second argument: data
    call outb            ; Call the outb function

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq

disk_handler:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Step 1: Set up the disk read operation
    mov rdi, 0x1F6       ; Command register port
    mov rsi, 0x00        ; Dummy value to clear out the port
    call outb            ; Send command to the port

    ; Step 2: Wait for the disk to be ready
.wait_for_ready:
    call inb_asm         ; Read the status register
    test al, 0x08        ; Check if the DRDY bit is set
    jz .wait_for_ready    ; Loop until the disk is ready

    ; Step 3: Send the read command
    mov rdi, 0x1F7       ; Command register port for reading
    mov rsi, 0x20        ; Read command (e.g., READ_SECTOR)
    call outb            ; Send read command

    ; Step 4: Read the data from the disk
    mov rdi, 0x1F0       ; Data register port
    mov rsi, buffer      ; Buffer to store read data
    mov rcx, 256         ; Number of words to read (512 bytes)
.read_data:
    call inb_asm         ; Read a word from the data register
    mov [rsi], ax        ; Store the word in the buffer
    add rsi, 2           ; Move to the next word in the buffer
    loop .read_data      ; Repeat for the specified number of words

    ; Step 5: Handle errors if necessary
    ; (Add error handling code based on your requirements)

    ; Step 6: Clean up and finish
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq
