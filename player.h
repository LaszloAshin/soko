/**
 * player.h
 */

#ifndef _player_H
#define _player_H	1

#ifndef _player_C
typedef struct _player_t player_t;
#endif /* _player_C */

#include "field.h"

player_t *new_player();
void free_player(player_t *this);

int player_move(player_t *this, int dir);
void player_setpos(player_t *this, field_t *field);
void player_keyboard(player_t *this, int key);
void player_resetcounters(player_t *this);
int player_moves(const player_t *this);
int player_pushes(const player_t *this);

#endif /* _player_H */

