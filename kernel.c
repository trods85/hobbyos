#include "efi.h"
#include <stdint.h>
#include "gfx.h"
#include "keyboard.h"
#include "game.h"
#include "timer.h"
#include "hud.h"
#include "ports.h"

extern char __bss_start[], __bss_end[], __stack_top[];

static BootInfo g_boot;
static Game g;
static uint64_t next_tick;

typedef enum { STATE_TITLE, STATE_PLAYING, STATE_DEAD } State;

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

static void exit_machine(void){
	outb(0x64, 0xFE);
	for(;;) __asm__ volatile("cli; hlt");
}

static UINT32 menu_move(UINT32 sel, Key k){
	if(k == KEY_UP) return (sel + MENU_ITEMS - 1) % MENU_ITEMS;
	if(k == KEY_DOWN) return (sel + 1) % MENU_ITEMS;
	return sel;
}

static void start_game(void){
	game_init(&g, &g_boot);
	game_draw_board(&g, &g_boot);
	hud_init(&g, &g_boot);
	next_tick = timer_ms() + game_tick_ms(&g);
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

	State state = STATE_TITLE;
	UINT32 sel = MENU_PLAY;
	show_splash(&g_boot, sel);
	for(;;){
		Key k = kbd_poll();
		switch(state){
			case STATE_TITLE: {
				UINT32 prev = sel;
				sel = menu_move(sel, k);
				if(sel != prev) splash_select(&g_boot, sel);

				if(k == KEY_ENTER){
					if (sel == MENU_EXIT) exit_machine();
					start_game();
					state = STATE_PLAYING;
				}
				break;
			}
			case STATE_PLAYING:{
				if(k != KEY_NONE) game_input(&g, k);
				
				uint64_t now = timer_ms();
				if(now >= next_tick){
					game_step(&g);
					if(g.alive){
						game_draw_update(&g, &g_boot);
						hud_update(&g, &g_boot);
						next_tick = now + game_tick_ms(&g);
					} else {
						sel = MENU_PLAY;
						game_draw_dead(&g, &g_boot, sel);
						state = STATE_DEAD;
					}
				}
				break;
			}
			case STATE_DEAD: {
				UINT32 prev = sel;
				sel = menu_move(sel, k);
				if(sel != prev) game_draw_dead_menu(&g_boot, sel);

				if (k == KEY_ENTER){
					if(sel == MENU_EXIT) exit_machine();
					start_game();
					state = STATE_PLAYING;
			}
			break;
		}
		}
	}
}


