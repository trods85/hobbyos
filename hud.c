#include "hud.h"
#include "gfx.h"

#define SCALE 3 // text scale: glyphs are 15 * 21 px
#define CHAR_W (6 * SCALE)
#define TEXT_H (7 * SCALE)
#define TEXT_Y ((HUD_H - TEXT_H)/2)


#define COLOR_BG 0x000A0A0A
#define COLOR_LABEL 0x00AAAAAA
#define COLOR_VALUE 0x0000FF00
#define COLOR_TRACK 0x00262626
#define COLOR_BAR_OK 0x0000CC00
#define COLOR_BAR_LOW 0x00FF4040

//Horizontal layout in pixels. Score, level, and bar. May add lives later if I decide to.
#define SCORE_X 16
#define SCORE_VAL_X (SCORE_X + 6 * CHAR_W) // SCORE 6 Chars
#define SCORE_VAL_W (6 * CHAR_W)

#define LEVEL_X (SCORE_VAL_X + SCORE_VAL_W + 32)
#define LEVEL_VAL_X (LEVEL_X + 6 * CHAR_W)
#define LEVEL_VAL_W (3 * CHAR_W)

#define HUNGER_X (LEVEL_VAL_X + LEVEL_VAL_W + 32) // HUNGER is 7 chars
#define BAR_X (HUNGER_X + 7 * CHAR_W)
#define BAR_W 200
#define BAR_H TEXT_H

static UINT32 shown_score, shown_level, shown_fill;

static void draw_value(BootInfo *bi, UINT32 x, UINT32 w, UINT32 value){
	fill_rect(bi, x, TEXT_Y, w, TEXT_H, COLOR_BG);
	draw_number(bi, value, x, TEXT_Y, SCALE, COLOR_VALUE);
}

void hud_init(const Game *g, BootInfo *bi){
	fill_rect(bi, 0, 0, bi->Width, HUD_H, COLOR_BG);

	draw_text(bi, "SCORE", SCORE_X, TEXT_Y, SCALE, COLOR_LABEL);
	draw_text(bi, "LEVEL", LEVEL_X, TEXT_Y, SCALE, COLOR_LABEL);
	draw_text(bi, "HUNGER", HUNGER_X, TEXT_Y, SCALE, COLOR_LABEL);

	shown_score = shown_level = shown_fill = 0xFFFFFFFF;
	hud_update(g, bi);
}

void hud_update(const Game *g, BootInfo *bi){
	if(g->score != shown_score){
		draw_value(bi, SCORE_VAL_X, SCORE_VAL_W, g->score);
		shown_score = g->score;
	}
	if (g->level != shown_level){
		draw_value(bi, LEVEL_VAL_X, LEVEL_VAL_W, g->level);
		shown_level = g->level;
	}

	UINT32 max = game_decay_ticks(g);
	UINT32 left = (max > g->hunger) ? max - g->hunger : 0;
	UINT32 fill = left * BAR_W / max;

	if (fill != shown_fill){
		UINT32 color = (fill < BAR_W / 4) ? COLOR_BAR_LOW : COLOR_BAR_OK;
		fill_rect(bi, BAR_X, TEXT_Y, fill, BAR_H, color);
		fill_rect(bi, BAR_X + fill, TEXT_Y, BAR_W - fill, BAR_H, COLOR_TRACK);
		shown_fill = fill;
	}
}

