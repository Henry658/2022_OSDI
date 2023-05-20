ARMGNU ?= aarch64-linux-gnu

COPS = -O0 -Wall -nostdlib -ffreestanding -nostdinc -nostartfiles -I include -g
ASMOPS = -I include -g

BUILD_DIR = build
SRC_DIR = src

all : kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img
run: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio -display none -dtb bcm2710-rpi-3-b-plus.dtb 
	
cpio: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio
	
sd: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -serial null -serial stdio -initrd ./initramfs.cpio -drive if=sd,file=./img/sfn_nctuos.img,format=raw

remote: all
	sudo chmod 777 /dev/ttyUSB0
	python3 sending.py
	screen /dev/ttyUSB0 115200

debug: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -S -s --dtb bcm2710-rpi-3-b-plus.dtb -serial null -serial stdio -initrd initramfs.cpio -drive if=sd,file=./img/sfn_nctuos.img,format=raw

gdb:
	aarch64-gdb build/kernel8.elf

filesystem:
	cd rootfs;find . | cpio -o -H newc > ../initramfs.cpio

filesystem2:
	cd rootfs;find . | cpio -o -H newc > ../INITRAMFS.CPIO
	
m:
	sudo cp /home/tzuheng/Downloads/sfn_nctuos.img /home/tzuheng/Desktop/OSDI/lab8/img/sfn_nctuos.img
	sudo losetup -fP img/sfn_nctuos.img
	sudo mount -t msdos /dev/loop10p1 img/temp
	sudo cp img/FAT_R.TXT img/temp/FAT_R.TXT
	
m1:
	sudo losetup -fP img/sfn_nctuos.img
	sudo mount -t msdos /dev/loop10p1 img/temp

um:
	sudo umount img/temp
	sudo losetup -d /dev/loop10
	
	


