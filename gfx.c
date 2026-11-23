#include "gfx.h"

static const char FONT_CHARS[] = "SNAKEHOBYRDCLVUG0123456789";

static const uint8_t FONT[][7] = {
	{0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, // S
	{0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}, // N
	{0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // A
	{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
	{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // E
	{0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // H
	{0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // O
	{0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // B
	{0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // Y
	{0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // R
	{0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}, // D
	{0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // C
	{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // L
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // V
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // U
	{0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // G
	{0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
	{0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
	{0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, // 2
	{0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // 3
	{0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
	{0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
	{0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
	{0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
	{0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
	{0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 9
};

/*
 * The following are the primitives (32 bit pixels, Pitch is in pixels not bytes.)
 */

// Simple helper to draw a pixel on a linear framebuffer (32 bit RGB/BRG format)
void draw_pixel(BootInfo *info, UINT32 x, UINT32 y, UINT32 color) {
	if (x >= info->Width || y >= info->Height) return;
	
	volatile UINT32 *fb = ( volatile UINT32 *)info->FrameBufferBase;
	UINTN offset = (UINTN)y * info->Pitch + x;
	fb[offset] = color;
}

// Draw a filled square block
void draw_box(BootInfo *info, UINT32 start_x, UINT32 start_y, UINT32 size, UINT32 color){
	for (UINT32 y = 0; y < size; y++) {
		for (UINT32 x = 0; x < size; x++) {
			draw_pixel(info, start_x + x, start_y + y, color);
		}
	}
}

// Fill the screen with pixels of given color
void fill_screen(BootInfo *info, UINT32 color){
	volatile UINT32 *fb = (volatile UINT32 *)info->FrameBufferBase;
	UINTN total = (UINTN)info->Pitch * info->Height;

	for (UINTN i = 0; i < total; i++){
		fb[i] = color;
	}
}

// Text drawing shit. 

void draw_text(BootInfo *info, const char *t, UINT32 x, UINT32 y, UINT32 scale, UINT32 color){
	for(; *t; t++, x+= 6 * scale) draw_char(info, *t, x, y, scale, color);
}

void draw_number(BootInfo *info, UINT32 v, UINT32 x, UINT32 y, UINT32 scale, UINT32 color){
	char buf[11];
	int n = 0;
	if(v == 0) buf[n++] = '0';
	while (v > 0 && n < 10) {buf[n++] = (char)('0' + v % 10); v/=10; }
	for (int i = n - 1; i >= 0; i--, x += 6 * scale)
		draw_char(info, buf[i], x, y, scale, color);
}

void draw_char(BootInfo *info, char c, UINT32 x, UINT32 y, UINT32 scale, UINT32 color){
	for (int i = 0; FONT_CHARS[i]; i++){
		if (FONT_CHARS[i] != c) continue;
		for(UINT32 row = 0; row < 7; row++){
			for (UINT32 col = 0; col < 5; col++){
				if(FONT[i][row] & (0x10 >> col)){
					draw_box(info, x + col * scale, y + row * scale, scale, color);
				}
			}
		}
		return;
	}
}

void draw_text_centered(BootInfo *info, const char *text, UINT32 y, UINT32 scale, UINT32 color){
	UINT32 n = 0;
	while (text[n]) n++;

	// each glyph is 5 columns plus 1 column of space 
	UINT32 width = n ? n * 6 * scale - scale : 0;
	UINT32 x = (width < info->Width) ? (info->Width - width)/2 : 0;

	for(UINT32 i = 0; i < n; i++, x += 6 * scale){
		draw_char(info, text[i], x, y, scale, color);
	}
}

void fill_rect(BootInfo *info, UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color){
	for (UINT32 j = 0; j < h; j++)
		for (UINT32 i = 0; i < w; i++)
			draw_pixel(info, x+i, y+j, color);
}
