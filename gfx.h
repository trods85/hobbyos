#ifndef GFX_H
#define GFX_H
#include "efi.h"
#include <stdint.h>

void draw_pixel(BootInfo *info, UINT32 x, UINT32 y, UINT32 color);
void draw_box(BootInfo *info, UINT32 x, UINT32 y, UINT32 size, UINT32 color);
void fill_screen(BootInfo *info, UINT32 color);
void draw_char(BootInfo *info, char c, UINT32 x, UINT32 y, UINT32 scale, UINT32 color);
void draw_text_centered(BootInfo *info, const char *t, UINT32 y, UINT32 scale, UINT32 color);
void show_splash(BootInfo *info);
void fill_rect(BootInfo *info, UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color);
void draw_text(BootInfo *info, const char *t, UINT32 x, UINT32 y, UINT32 scale, UINT32 color);
void draw_number(BootInfo *info, UINT32 v, UINT32 x, UINT32 y, UINT32 scale, UINT32 color);

#endif
