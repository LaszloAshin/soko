/**
 * player.c
 */

#define _player_C	1

#include <malloc.h>

#include "field.h"

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

