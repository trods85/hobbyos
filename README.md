# HobbyOS Snake

A snake game that runs on bare metal. There is no operating system underneath it:
a small UEFI bootloader loads a freestanding x86_64 kernel, and the kernel draws
straight to the framebuffer and reads the keyboard hardware directly.

Written in C as a learning project in low-level x86_64 and UEFI.

<!-- Add a screenshot here: ![HobbyOS Snake](docs/screenshot.png) -->

## Features

- Custom UEFI bootloader (no gnu-efi, no EDK2 libraries) with its own `efi.h`
- Flat-binary kernel loaded at `0x100000`
- Linear framebuffer graphics with a built-in 5x7 bitmap font
- PS/2 keyboard driver (polling, scancode set 1)
- Timer calibrated from the CPU timestamp counter against the PIT
- Snake with a subtle grid, five fruit on screen, levels, and a HUD (score, level, hunger bar)

## How the game plays

- Move with the **arrow keys** or **WASD**. Press **Enter** to start and to restart after dying.
- The snake keeps moving in its current direction until you turn it. 180 degree turns are ignored.
- **The edges wrap around.** The only way to die is to run into yourself.
- Eating fruit grows the snake and adds to your score.
- Every 5 fruit raises the **level**, which speeds the game up (200 ms per step at level 0,
  12 ms faster per level, with a floor of 60 ms).
- **Hunger:** the snake slowly loses tail segments, down to a minimum length of 4. Eating resets
  the hunger bar, and higher levels drain it faster. This keeps the snake from ever filling the screen.

## How it boots

```
UEFI firmware
  -> BOOTX64.EFI (boot.c)
       1. open the boot volume and read KERNEL.BIN
       2. allocate memory at 0x100000 (kernel + room for .bss and stack) and load it
       3. find the GOP framebuffer and check its pixel format
       4. get the memory map and call ExitBootServices
       5. jump to the kernel, passing a BootInfo struct
  -> KERNEL.BIN (kernel.c)
       1. switch to its own stack and clear .bss
       2. copy BootInfo off the firmware's stack
       3. init keyboard and timer, show the splash, run the game loop
```

`BootInfo` carries the framebuffer address, resolution, pitch, pixel format, and the UEFI memory map.

## Project layout

| File | Purpose |
|---|---|
| `boot.c` | UEFI bootloader |
| `efi.h` | Minimal UEFI type and protocol definitions, plus the `BootInfo` struct |
| `kernel.c` | Kernel entry (`_start`), setup, and the main game loop |
| `linker.ld` | Linker script: load address, `.bss` and stack layout |
| `gfx.c` / `gfx.h` | Pixels, rectangles, bitmap font, text and number drawing |
| `splash.c` | Title screen |
| `game.c` / `game.h` | Game rules and board drawing |
| `hud.c` / `hud.h` | Score, level and hunger bar |
| `keyboard.c` / `keyboard.h` | PS/2 keyboard polling |
| `timer.c` / `timer.h` | TSC timer, calibrated against the PIT |
| `ports.h` | `inb` / `outb` helpers |
| `Makefile` | Build and run |

## Building

Requirements (package names are for Arch Linux):

- `clang`, `lld`, `llvm` (for `llvm-objcopy`)
- `qemu-system-x86`
- `edk2-ovmf`

Check that `OVMF_CODE` and `OVMF_VARS` in the `Makefile` point at your OVMF files.

```sh
make          # builds esp/EFI/BOOT/BOOTX64.EFI and esp/KERNEL.BIN
make run      # builds, then boots in QEMU with OVMF
make clean
```

`make run` uses a q35 machine with 256 MB of RAM and exposes the `esp/` directory as a FAT drive.
Click inside the QEMU window before pressing keys so it receives the keyboard.

## Debugging

Add `-s -S` to the QEMU command in the `Makefile` to make QEMU wait for GDB, then:

```
gdb
(gdb) file kernel.elf
(gdb) set architecture i386:x86-64
(gdb) target remote :1234
(gdb) hbreak _start
(gdb) continue
```

Use a hardware breakpoint (`hbreak`), because the kernel is copied into RAM after the VM starts.
Useful extra QEMU flags: `-d int,cpu_reset -D qemu.log -no-reboot -no-shutdown` to log exceptions.

To read the machine code: `objdump -d -M intel kernel.elf`.

## Known limitations

- **Keyboard:** PS/2 only. Works in QEMU and on machines with a PS/2 internal keyboard.
  USB keyboards would need a USB (xHCI) driver.
- **Fixed load address:** the kernel is loaded at exactly `0x100000`. Some real firmware
  already uses that region, and loading will fail there.
- **Pixel format:** only 32-bit RGB and BGR framebuffers are supported.
- **No interrupts:** everything is polled. Interrupts are disabled after boot.

## Roadmap

- [x] Bootloader, memory map, `ExitBootServices`
- [x] Framebuffer graphics and bitmap font
- [x] Keyboard input and timing
- [x] Snake game with levels, hunger, and multiple fruit
- [x] HUD
- [ ] Main menu (play / exit)
- [ ] Bootable USB image (GPT + FAT32 EFI System Partition)
- [ ] Handle a fixed-load-address failure on real hardware

## License

Not yet chosen.
