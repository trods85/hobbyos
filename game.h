#ifndef GAME_H
#define GAME_H

#include "efi.h"
#include "keyboard.h"
#include <stdint.h>

#define CELL		20 // pixels per grid cell
#define HUD_H		48
#define MAX_SNAKE	1024
#define MIN_LEN		4
#define FRUITS_PER_LVL 	5
#define FRUIT_COUNT	5

typedef struct {UINT32 x, y;} Cell; // grid coords

typedef struct {
	Cell body[MAX_SNAKE];
	UINT32 head, length;

	Cell fruit[FRUIT_COUNT];
	int dir_x, dir_y;
	int next_x, next_y;

	UINT32 cols, rows;
	UINT32 org_x, org_y;

	UINT32 score, level;
	UINT32 hunger;
	int alive;

	uint64_t rng;

	Cell erased[2];
	UINT32 erased_n;
} Game;

void game_init(Game *g, BootInfo *bi);
void game_input(Game *g, Key k);
void game_step(Game *g);
UINT32 game_tick_ms(const Game *g);
UINT32 game_decay_ticks(const Game *g);

void game_draw_board(const Game *g, BootInfo *bi); // Full redraw at start of the game.
void game_draw_update(const Game *g, BootInfo *bi); // Update only changed things
void game_draw_dead(const Game *g, BootInfo *bi);

#endif // GAME_H
