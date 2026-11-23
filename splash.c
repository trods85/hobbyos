#include "gfx.h"

#define COLOR_BG 0x001A1A1A
#define COLOR_TITLE 0x0000FF00
#define COLOR_SUBTITLE 0x00888888
#define COLOR_SNAKE 0x00008800
#define COLOR_HEAD 0x0000FF00

void show_splash(BootInfo *info){
	const UINT32 title_scale = 12;
	const UINT32 sub_scale = 4;
	const UINT32 seg_size = 20;
	const UINT32 seg_step = 24;
	const UINT32 seg_count = 6;

	UINT32 title_y = info->Height/3;
	UINT32 sub_y = title_y + 7 * title_scale + 24;
	UINT32 snake_y = sub_y + 7 * sub_scale + 32;

	fill_screen(info, COLOR_BG);

	draw_text_centered(info, "SNAKE", title_y, title_scale, COLOR_TITLE);
	draw_text_centered(info, "HOBBYOS", sub_y, sub_scale, COLOR_SUBTITLE);

	// Draw a simple snake head and body
	UINT32 snake_w = seg_count * seg_step - (seg_step - seg_size);
	UINT32 snake_x = (snake_w < info->Width)?(info->Width - snake_w)/2 : 0;

	for(UINT32 i = 0; i < seg_count; i++){
		UINT32 color = (i == seg_count - 1) ? COLOR_HEAD:COLOR_SNAKE;
		draw_box(info, snake_x + i * seg_step, snake_y, seg_size, color);
	}
}
