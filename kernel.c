#include "efi.h"
#include <stdint.h>
#include "gfx.h"
#include "keyboard.h"
#include "game.h"
#include "timer.h"
#include "hud.h"

extern char __bss_start[], __bss_end[], __stack_top[];
static BootInfo g_boot;
void kmain(BootInfo *bi);

// Simple helper to draw a pixel on a linear framebuffer (32 bit RGB/BRG format)

__attribute__((naked,section(".text.entry")))
void _start(void) {
	//Clear Screen
	__asm__ volatile (
			"cli\n"
			"lea __stack_top(%rip), %rsp\n"
			"xor %ebp, %ebp\n"
			"call kmain\n"
			"1: hlt\n"
			"jmp 1b\n"
			);
	
	
}

void kmain(BootInfo *bi){
	for (volatile char *p = __bss_start; p < __bss_end; p++) *p = 0;
	g_boot.FrameBufferBase 	= bi->FrameBufferBase;
	g_boot.Width		= bi->Width;
	g_boot.Height		= bi->Height;
	g_boot.Pitch		= bi->Pitch;
	g_boot.MemoryMap	= bi->MemoryMap;
	g_boot.MemoryMapSize	= bi->MemoryMapSize;
	g_boot.DescriptorSize	= bi->DescriptorSize;
	g_boot.PixelFormat	= bi->PixelFormat;

	kbd_init();
	timer_init();
	show_splash(&g_boot);
	while(kbd_poll() != KEY_ENTER) {} // wait on the splash

	static Game g;

	for(;;) {
		game_init(&g, &g_boot);
		game_draw_board(&g, &g_boot);
		hud_init(&g, &g_boot);

		uint64_t next = timer_ms() + game_tick_ms(&g);
		while (g.alive){
			Key k = kbd_poll();	
			if(k != KEY_NONE) game_input(&g, k);

			uint64_t now = timer_ms();
			if (now >= next) {
				game_step(&g);
				if (g.alive) {game_draw_update(&g, &g_boot); hud_update(&g, &g_boot);}
				next = now + game_tick_ms(&g); // no catch-up bursts after a slow fram
			}
		}
	game_draw_dead(&g, &g_boot);
	while (kbd_poll() != KEY_ENTER){}
	}
}
