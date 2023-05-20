aarch64-linux-gnu-gcc -c user.S
aarch64-linux-gnu-ld -T linker.ld -o user.elf user.o
aarch64-linux-gnu-objcopy -O binary user.elf user.img
