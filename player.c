/**
 * player.c
 */

#define _player_C	1

#include <malloc.h>
#include <SDL/SDL_keysym.h>

#include "field.h"
#include "main.h"

typedef struct {
	field_t *field;
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
	field_t *neigh = field_playermove(this->field, dir);
	if (neigh != NULL) {
		player_setpos(this, neigh);
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
player_keyboard(player_t *this, map_t *map, int key)
{
	int bo = 0;
	switch (key) {
		case SDLK_UP:
			bo = player_move(this, FIELD_UP);
			break;
		case SDLK_RIGHT:
			bo = player_move(this, FIELD_RIGHT);
			break;
		case SDLK_DOWN:
			bo = player_move(this, FIELD_DOWN);
			break;
		case SDLK_LEFT:
			bo = player_move(this, FIELD_LEFT);
			break;
		case SDLK_ESCAPE:
			main_switchtomenu();
			break;
		case SDLK_RETURN:
			/* undo last move... */
			break;
	}
	if (bo) bo = map_isdone(map);
	if (bo) {
		main_nextmap();
		main_switchtogame();
	}
}

