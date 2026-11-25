#include "gfx.h"

#define COLOR_BG 0x001A1A1A
#define COLOR_TITLE 0x0000FF00
#define COLOR_SUBTITLE 0x00888888
#define COLOR_SNAKE 0x00008800
#define COLOR_HEAD 0x0000FF00
#define COLOR_ITEM_ON 0x0000FF00
#define COLOR_ITEM_OFF 0x0666666

#define TITLE_SCALE 12
#define SUB_SCALE 4
#define SEG_SIZE 20
#define SEG_STEP 24
#define SEG_COUNT 6
#define MENU_SCALE 4

static const char *const ITEMS[MENU_ITEMS] = {"PLAY", "EXIT"};

// Vertical layout computed in one place so the full draw and the selection-only redraw always agree
//
static UINT32 title_y(const BootInfo *info){return info->Height/3;}
static UINT32 sub_y(const BootInfo *info){return title_y(info) + 7 * TITLE_SCALE + 24;}
static UINT32 snake_y(const BootInfo *info){return sub_y(info) + 7 * SUB_SCALE + 32;}
static UINT32 menu_y(const BootInfo *info){return snake_y(info) + SEG_SIZE + 40;}

void splash_select(BootInfo *info, UINT32 selected){
	draw_menu(info, ITEMS, MENU_ITEMS, selected, menu_y(info), MENU_SCALE, COLOR_ITEM_ON, COLOR_ITEM_OFF, COLOR_BG);
}

void show_splash(BootInfo *info, UINT32 selected){
	fill_screen(info, COLOR_BG);

	draw_text_centered(info, "SNAKE", title_y(info), TITLE_SCALE, COLOR_TITLE);
	draw_text_centered(info, "HOBBYOS", sub_y(info), SUB_SCALE, COLOR_SUBTITLE);

	// Draw a simple snake head and body
	UINT32 snake_w = SEG_COUNT * SEG_STEP - (SEG_STEP - SEG_SIZE);
	UINT32 snake_x = (snake_w < info->Width)?(info->Width - snake_w)/2 : 0;

	for(UINT32 i = 0; i < SEG_COUNT; i++){
		UINT32 color = (i == SEG_COUNT - 1) ? COLOR_HEAD:COLOR_SNAKE;
		draw_box(info, snake_x + i * SEG_STEP, snake_y(info), SEG_SIZE, color);
	}
	splash_select(info, selected);
}
