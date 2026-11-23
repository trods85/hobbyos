CC = clang
LD = ld.lld
OBJCOPY = llvm-objcopy

ESP = esp
OVMF_CODE = /usr/share/edk2/x64/OVMF_CODE.4m.fd
OVMF_VARS = /usr/share/edk2/x64/OVMF_VARS.4m.fd

all: $(ESP)/EFI/BOOT/BOOTX64.EFI $(ESP)/KERNEL.BIN

# 1. Build UEFI Bootloader (Windows PE32+ target)
$(ESP)/EFI/BOOT/BOOTX64.EFI: boot.c efi.h
	mkdir -p $(ESP)/EFI/BOOT
	$(CC) -target x86_64-unknown-windows -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone -Wall -Wextra -std=c17 -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld boot.c -o $@

# 2. Build Flat Binary Kernel (Freestanding x86_64 ELF -> RAW BINARY)
KERNEL_SRC = kernel.c gfx.c splash.c keyboard.c game.c timer.c hud.c
KERNEL_OBJ = $(KERNEL_SRC:.c=.o)
KFLAGS =  -target x86_64-unknown-none-elf -ffreestanding -fno-stack-protector -mno-red-zone -mno-sse -mno-sse2 -mno-mmx -Wall -Wextra -std=c17 -c
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(KFLAGS) -c $< -o $@

$(ESP)/KERNEL.BIN: $(KERNEL_OBJ) linker.ld
	mkdir -p $(ESP)
	$(LD) -T linker.ld -nostdlib $(KERNEL_OBJ) -o kernel.elf
	$(OBJCOPY) -O binary kernel.elf $@
# 3. Boot into QEMU with OVMF
run: all
	cp -n $(OVMF_VARS) OVMF_VARS.fd
	qemu-system-x86_64 -machine q35 -m 256 	-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) -drive if=pflash,format=raw,file=OVMF_VARS.fd -drive format=raw,file=fat:rw:$(ESP) -net none

clean:
	rm -rf $(ESP) *.o kernel.elf OVMF_VARS.fd
