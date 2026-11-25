#include "game.h"
#include "gfx.h"
#include "timer.h"

#define COLOR_GAP 0x00141414 //Grid color
#define COLOR_EMPTY 0x001C1C1C
#define COLOR_BODY 0x0000AA00
#define COLOR_HEAD 0x0000FF00
#define COLOR_FRUIT 0x00FF3030
#define COLOR_DEAD 0x00AA2020
#define DEAD_TITLE_SCALE 8
#define DEAD_MENU_SCALE 4
#define DEAD_PAD 24
#define DEAD_BG 0x00101010
#define DEAD_BH (DEAD_PAD + 7 * DEAD_TITLE_SCALE + 24 + MENU_ITEMS * 10 * DEAD_MENU_SCALE + DEAD_PAD)

static const char *const DEAD_ITEMS[MENU_ITEMS] = {"PLAY AGAIN", "EXIT"};

//
// HELPERS
//

static UINT32 rng_next(Game *g) {
	uint64_t x = g->rng;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	g->rng = x;
	return(UINT32)(x >> 16);
}

//i = 0 is the head, i = lenght-1 is the tail
static Cell segment(const Game *g, UINT32 i) {
	return g->body[(g->head + MAX_SNAKE - i) % MAX_SNAKE];
}

static int on_snake(const Game *g, UINT32 x, UINT32 y){
	for(UINT32 i = 0; i < g->length; i++){
		Cell c = segment(g, i);
		if(c.x == x && c.y == y) return 1;
	}
	return 0;
}
static int on_fruit(const Game *g, UINT32 x, UINT32 y, UINT32 skip){
	for(UINT32 i = 0; i < FRUIT_COUNT; i ++){
		if(i == skip) continue;
		if(g->fruit[i].x == x && g->fruit[i].y == y) return 1;
	}
	return 0;
}

static void place_fruit(Game *g, UINT32 i){
	for( int tries = 0; tries < 64; tries++ ){
		UINT32 x = rng_next(g) % g->cols;
		UINT32 y = rng_next(g) % g->rows;
			if(!on_snake(g, x, y) && !on_fruit(g, x, y, i)){
				g->fruit[i].x = x; g->fruit[i].y = y; return;
			}
		}
	
	for(UINT32 y = 0; y < g->rows; y++){
		for( UINT32 x = 0; x < g->cols; x++){
			if(!on_snake(g, x, y) && !on_fruit(g, x, y, i)){g->fruit[i].x = x; g->fruit[i].y = y; return;}
		}
	}
// IF the board is full leave the fruit where it is
}

UINT32 game_decay_ticks(const Game * g) {
	UINT32 base = 30, step = 2, floor = 12;
	return(g->level * step < base - floor)?base - g->level * step : floor;
}

//
// Public RULES
//
void game_init(Game *g, BootInfo *bi) {
	g->cols = bi->Width / CELL;
	g->rows = (bi->Height - HUD_H)/ CELL;
	g->org_x = (bi->Width - g->cols * CELL) / 2;
	g->org_y = HUD_H + (bi->Height - HUD_H - g->rows * CELL) / 2;

	g->rng = timer_rdtsc() | 1;

	g->length = MIN_LEN;
	g->head = MIN_LEN - 1;
	for(UINT32 i = 0; i < MIN_LEN; i++){
		g->body[i].x = g->cols / 2-( MIN_LEN - 1 ) + i;
		g->body[i].y = g->rows / 2;
	}
	g->dir_x = 1; g->dir_y = 0;
	g->next_x = 1; g->next_y = 0;

	g->score = 0;
	g->level = 0;
	g->hunger = 0;
	g->alive = 1;
	g->erased_n = 0;

	for(UINT32 i = 0; i < FRUIT_COUNT; i++){g->fruit[i].x = g->cols; g->fruit[i].y = g->rows;}
	for(UINT32 i = 0; i < FRUIT_COUNT; i++) place_fruit(g, i);
}

void game_input(Game *g, Key k){
	int nx = g->next_x, ny = g->next_y;

	if(k == KEY_UP) {nx = 0; ny = -1;}
	else if (k == KEY_DOWN) {nx = 0; ny = 1;}
	else if (k == KEY_LEFT) {nx = -1; ny = 0;}
	else if (k == KEY_RIGHT) {nx = 1; ny = 0;}
	else return;

	if(nx == -g->dir_x && ny == -g->dir_y) return;

	g->next_x = nx;
	g->next_y = ny;
}

UINT32 game_tick_ms(const Game *g){
	UINT32 base = 200, step = 12, floor = 60;
	return(g->level * step < base - floor) ? base - g->level * step : floor;
}

void game_step(Game * g) {
	g->erased_n = 0;
	g->dir_x = g->next_x;
	g->dir_y = g->next_y;

	Cell h = segment(g, 0);

	// move with wrap-around (cast to int first: dir can be -1)
	UINT32 nx = (UINT32)((int)h.x + g->dir_x + (int)g->cols) % g->cols;
	UINT32 ny = (UINT32)((int)h.y + g->dir_y + (int)g->rows) % g->rows;

	int eaten = -1;
	for(UINT32 i = 0; i < FRUIT_COUNT; i++)
		if(nx == g->fruit[i].x && ny == g->fruit[i].y) {eaten = (int)i; break;}
	int eating = (eaten >= 0);
	Cell tail = segment(g, g->length - 1);

	for(UINT32 i = 0; i < g->length; i++){
		if(i==g->length - 1 && !eating)continue;
		Cell c = segment(g, i);
		if(c.x == nx && c.y == ny) {g->alive = 0; return;}

	}
	g->head = ( g->head + 1) % MAX_SNAKE;
	g->body[g->head].x = nx;
	g->body[g->head].y = ny;

	if(eating){
		if(g->length < MAX_SNAKE - 1)g->length++;
		g->score++;
		g->level = g->score / FRUITS_PER_LVL;
		g->hunger = 0;
		place_fruit(g, (UINT32) eaten);
	} else {
		g->erased[g->erased_n++] = tail;
	}
	if(++g->hunger >= game_decay_ticks(g)) {
		g->hunger = 0;
		if(g->length > MIN_LEN) {
			g->erased[g->erased_n++] = segment(g, g->length - 1); g->length--;
		}
	}
}

//
// Public Drawing
//

static void draw_cell(const Game *g, BootInfo *bi, UINT32 cx, UINT32 cy, UINT32 color){
	draw_box(bi, g->org_x + cx * CELL, g->org_y + cy * CELL, CELL - 1, color);
}

void game_draw_board(const Game *g, BootInfo *bi){
	fill_screen(bi, COLOR_GAP);

	for(UINT32 y = 0; y < g->rows; y++)
		for(UINT32 x = 0; x < g->cols; x++)
			draw_cell(g, bi, x, y, COLOR_EMPTY);


	for(UINT32 i = 1; i < g->length; i++) {
		Cell c = segment(g, i);
		draw_cell(g, bi, c.x, c.y, COLOR_BODY);

	}
	Cell h = segment(g, 0);
	draw_cell(g, bi, h.x, h.y, COLOR_HEAD);
	for(UINT32 i = 0; i < FRUIT_COUNT; i++)
		if(g->fruit[i].x < g->cols)
			draw_cell(g, bi, g->fruit[i].x, g->fruit[i].y, COLOR_FRUIT);
}

void game_draw_update(const Game *g, BootInfo *bi){
	// 1. Freed cells go back to empty, before drawing the head, it may reuse the tail cell.
	for(UINT32 i = 0; i < g->erased_n; i++)
		draw_cell(g, bi, g->erased[i].x, g->erased[i].y, COLOR_EMPTY);
	// 2. Old head becomes body, new head is bright.
	if(g->length > 1){
		Cell old = segment(g, 1);
		draw_cell(g, bi, old.x, old.y, COLOR_BODY);
	}
	Cell h = segment(g, 0);
	draw_cell(g, bi, h.x, h.y, COLOR_HEAD);

	// 3. fruit(cheap, and covers the case where it just moved)
	for(UINT32 i = 0; i < FRUIT_COUNT; i++)
		if(g->fruit[i].x < g->cols)
			draw_cell(g, bi, g->fruit[i].x, g->fruit[i].y, COLOR_FRUIT);
}

static UINT32 dead_top(const BootInfo *bi) {
	return (bi->Height > DEAD_BH) ? (bi->Height - DEAD_BH)/2:0;
}

void game_draw_dead_menu(BootInfo *bi, UINT32 selected){
	UINT32 y = dead_top(bi) + DEAD_PAD + 7 * DEAD_TITLE_SCALE + 24;
	draw_menu(bi, DEAD_ITEMS, MENU_ITEMS, selected, y, DEAD_MENU_SCALE, COLOR_HEAD, 0x00777777, DEAD_BG);
}


void game_draw_dead(const Game *g, BootInfo *bi, UINT32 selected){
	for(UINT32 i = 0; i < g->length; i++){
		Cell c = segment(g, i);
		draw_cell(g, bi, c.x, c.y, COLOR_DEAD);
	}
	UINT32 bw = 9 * 6 * DEAD_TITLE_SCALE - DEAD_TITLE_SCALE + 2 * DEAD_PAD;
	UINT32 top = dead_top(bi);
	UINT32 bx = (bi->Width > bw) ? (bi->Width - bw)/2:0;

	fill_rect(bi, bx, top, bw, DEAD_BH, DEAD_BG);
	draw_text_centered(bi, "SNERK DED", top + DEAD_PAD, DEAD_TITLE_SCALE, 0x00FF4040);
	game_draw_dead_menu(bi, selected);
}
