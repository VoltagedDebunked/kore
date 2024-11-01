AS = nasm
CC = gcc
LD = ld
CFLAGS = -m64 -ffreestanding -O3

all: kore

kernel.bin: src/kernel.o src/io.o src/gdt.o src/idt.o src/memory.o src/drivers/timer.o src/drivers/keyboard.o src/drivers/disk.o src/interrupts.o src/drivers/firmware.o
	$(LD) -T scripts/linker.ld -o kernel.bin src/kernel.o src/io.o src/gdt.o src/idt.o src/memory.o src/drivers/timer.o src/drivers/keyboard.o src/drivers/disk.o src/drivers/firmware.o src/interrupts.o

src/kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c src/kernel.c -o src/kernel.o

src/io.o: src/io.c
	$(CC) $(CFLAGS) -c src/io.c -o src/io.o

src/gdt.o: src/gdt.c
	$(CC) $(CFLAGS) -c src/gdt.c -o src/gdt.o

src/idt.o: src/idt.c
	$(CC) $(CFLAGS) -c src/idt.c -o src/idt.o

src/memory.o: src/memory.c
	$(CC) $(CFLAGS) -c src/memory.c -o src/memory.o

src/drivers/timer.o: src/drivers/timer.c
	$(CC) $(CFLAGS) -c src/drivers/timer.c -o src/drivers/timer.o

src/drivers/keyboard.o: src/drivers/keyboard.c
	$(CC) $(CFLAGS) -c src/drivers/keyboard.c -o src/drivers/keyboard.o

src/drivers/disk.o: src/drivers/disk.c
	$(CC) $(CFLAGS) -c src/drivers/disk.c -o src/drivers/disk.o

src/drivers/firmware.o: src/drivers/firmware.c
	$(CC) $(CFLAGS) -c src/drivers/firmware.c -o src/drivers/firmware.o

src/interrupts.o: src/interrupts.asm
	$(AS) src/interrupts.asm -f elf64 -o src/interrupts.o

kore: src/boot.asm kernel.bin
	mkdir -p build
	$(AS) src/boot.asm -f elf64 -o boot.bin
	cat boot.bin kernel.bin > build/kore

clean:
	rm -rf src/*.o src/drivers/*.o *.bin build

docs:
	mkdir -p docs
	doxygen Doxyfile 

docs-clean:
	rm -rf docs
