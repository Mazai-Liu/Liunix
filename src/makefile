BUILD:=../build
SRC:=.
ENTRYPOINT:=0x10000

INCLUDE:=-I$(SRC)/include
CFLAGS:= -m32
CFLAGS+= -fno-builtin # 不需要gcc的内置函数
CFLAGS+= -nostdinc  # 不需要标准头文件
CFLAGS+= -fno-pic  # 不需要位置无关代码
CFLAGS+= -fno-pie  # 不需要位置无关的可执行程序
CFLAGS+= -nostdlib  # 不需要标准库
CFLAGS+= -fno-stack-protector  # 不需要栈保护
CFLAGS:=$(strip ${CFLAGS})
DEBUG:= -g

$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@
$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -gdwarf -f elf32 $(DEBUG) $< -o $@
$(BUILD)/%.o: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

# 链接
$(BUILD)/kernel.bin: $(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/handler.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/kernel/rtc.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/vsprintf.o \
	$(BUILD)/lib/stdlib.o \
	
	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
					$(BUILD)/boot/loader.bin \
					$(BUILD)/system.bin \
					$(BUILD)/system.map            
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

test: $(BUILD)/master.img

.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -unlock

.PHONY: clean
clean:
	rm -rf $(BUILD)


QEMU:= qemu-system-i386 -m 32M -boot c \
	-drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw \
	-audiodev pa,id=hda \
	-machine pcspk-audiodev=hda \
	-rtc base=localtime \

.PHONY: qemu
qemu: $(BUILD)/master.img
# qemu-system-i386 -m 32M -boot c -hda $<
	$(QEMU)

.PHONY: qemug
qemug: $(BUILD)/master.img
	$(QEMU) -s -S


${BUILD}/master.vmdk: $(BUILD)/master.img
	qemu-img convert -pO vmdk $< $@

.PHONY: vmdk
vmdk: ${BUILD}/master.vmdk
	