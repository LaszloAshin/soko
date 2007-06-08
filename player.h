/**
 * player.h
 */

#ifndef _player_H
#define _player_H	1

#include "field.h"

#ifndef _player_C
typedef struct _player_t player_t;
#endif /* _player_C */

player_t *new_player();
void free_player(player_t *this);

int player_move(player_t *this, int dir);
void player_setpos(player_t *this, field_t *field);

#endif /* _player_H */

