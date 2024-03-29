/**
 * player.c
 */

#define _player_C	1

#include <malloc.h>
#include <SDL/SDL_keysym.h>

#include "main.h"

typedef struct {
	struct _field_t *field;
	int moves, pushes;
} player_t;

#include "player.h"

player_t *
new_player()
{
	player_t *this;

	this = (player_t *)malloc(sizeof(player_t));
	if (this == NULL) {
		return this;
	}
	this->field = NULL;
	return this;
}

void
free_player(player_t *this)
{
	if (this == NULL) {
		return;
	}
	free(this);
}

int
player_move(player_t *this, int dir)
{
	int i;
	field_t *neigh = field_playermove(this->field, dir, &i);
	if (neigh != NULL) {
		player_setpos(this, neigh);
		++this->moves;
		if (i > 1) {
			++this->pushes;
		}
		return 1;
	}
	return 0;
}

void
player_setpos(player_t *this, field_t *field)
{
	this->field = field;
}

void
player_keyboard(player_t *this, int key)
{
	switch (key) {
		case SDLK_UP:
			player_move(this, FIELD_UP);
			break;
		case SDLK_RIGHT:
			player_move(this, FIELD_RIGHT);
			break;
		case SDLK_DOWN:
			player_move(this, FIELD_DOWN);
			break;
		case SDLK_LEFT:
			player_move(this, FIELD_LEFT);
			break;
		case SDLK_ESCAPE:
			main_switchtomenu();
			break;
		case SDLK_RETURN:
			/* undo last move... */
			break;
	}
}

void
player_resetcounters(player_t *this)
{
	this->pushes = this->moves = 0;
}

int
player_moves(const player_t *this)
{
	return this->moves;
}

int
player_pushes(const player_t *this)
{
	return this->pushes;
}

