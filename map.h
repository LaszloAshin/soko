/**
 * map.h
 */

#ifndef _map_H
#define _map_H	1

#ifndef _map_C
typedef struct _map_t map_t;
#endif /* _map_C */

#include "player.h"

map_t *new_map(int num, player_t *player);
void free_map(map_t *this);

void map_draw(map_t *this);
int map_isdone(map_t *this);
int map_getnum(map_t *this);
int map_count();

#endif /* _map_H */

