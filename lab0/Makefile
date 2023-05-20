GNUgcc = aarch64-linux-gnu-gcc #compiler
GNUlinker = aarch64-linux-gnu-ld
GNUobjcopy = aarch64-linux-gnu-objcopy

kernel8.img: kernel8.elf
	$(GNUobjcopy) -O binary kernel8.elf kernel8.img

kernel8.elf: start.o linker.ld
	$(GNUlinker) -T linker.ld -o kernel8.elf start.o

start.o: start.S
	$(GNUgcc) -c start.S
