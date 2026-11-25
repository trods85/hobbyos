#ifndef GFX_H
#define GFX_H
#include "efi.h"
#include <stdint.h>

enum {MENU_PLAY = 0, MENU_EXIT = 1, MENU_ITEMS = 2};

void draw_menu(BootInfo *bi, const char *const *items, UINT32 n, UINT32 selected, UINT32 y, UINT32 scale, UINT32 on, UINT32 off, UINT32 bg);

void draw_pixel(BootInfo *info, UINT32 x, UINT32 y, UINT32 color);
void draw_box(BootInfo *info, UINT32 x, UINT32 y, UINT32 size, UINT32 color);
void fill_screen(BootInfo *info, UINT32 color);
void draw_char(BootInfo *info, char c, UINT32 x, UINT32 y, UINT32 scale, UINT32 color);
void draw_text_centered(BootInfo *info, const char *t, UINT32 y, UINT32 scale, UINT32 color);
void show_splash(BootInfo *info, UINT32 selected);
void splash_select(BootInfo *info, UINT32 selected);
void fill_rect(BootInfo *info, UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color);
void draw_text(BootInfo *info, const char *t, UINT32 x, UINT32 y, UINT32 scale, UINT32 color);
void draw_number(BootInfo *info, UINT32 v, UINT32 x, UINT32 y, UINT32 scale, UINT32 color);

#endif
