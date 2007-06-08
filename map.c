/**
 * map.c
 */

#define _map_C	1

#include <malloc.h>
#include <stdio.h>

#include "list.h"
#include "field.h"
#include "gr.h"
#include "player.h"

typedef struct {
	int num;
	list_t *fields;
} map_t;

#include "map.h"

field_t *
map_getfieldforcoords(map_t *this, int x, int y)
{
	int cx, cy;
	field_t *field;
	list_iterator_t *i = new_list_iterator(this->fields);
	while ((field = list_iterator_current(i)) != NULL) {
		field_getcoords(field, &cx, &cy);
		if (cx == x && cy == y) {
			break;
		}
		list_iterator_next(i);
	}
	free_list_iterator(i);
	return field;
}

map_t *
new_map(int num, player_t *player)
{
	map_t *this;
	char fname[8]; /* "maps/xx" */
	int i, ch;
	FILE *f;
	int x, y;
	int w;

	i = snprintf(fname, sizeof(fname), "maps/%02x", num);
	if (i < 0 || i >= sizeof(fname)) {
		return NULL;
	}
	f = fopen(fname, "r");
	if (f == NULL) {
		return NULL;
	}
	this = (map_t *)malloc(sizeof(map_t));
	if (this == NULL) {
		return this;
	}
	this->num = num;
	this->fields = new_list((void (*)(void *))free_field, 0);
	w = x = y = 0;
	while ((ch = fgetc(f)) != EOF) {
		i = -1;
		switch (ch) {
			case '\n':
				++y;
				if (x > w) w = x;
				x = -1;
				break;
			case ' ':
				break;
			case '#':
				i = FIELD_WALL;
				break;
			case '.':
				i = FIELD_FREE;
				break;
			case 'S':
				i = FIELD_FREE | FIELD_PLAYER;
				break;
			case 'o':
				i = FIELD_FREE | FIELD_BOX;
				break;
			case 'x':
				i = FIELD_FREE | FIELD_DEST;
				break;
			default:
				break;
		}
		if (i >= 0) {
			field_t *neigh;
			field_t *field = new_field(x, y, i);
			list_insert(this->fields, field);
			if (i & FIELD_PLAYER) {
				player_setpos(player, field);
			}
			neigh = map_getfieldforcoords(this, x, y - 1);
			if (neigh != NULL) {
				field_addneigh(field, FIELD_UP, neigh);
				field_addneigh(neigh, FIELD_DOWN, field);
			}
			neigh = map_getfieldforcoords(this, x - 1, y);
			if (neigh != NULL) {
				field_addneigh(field, FIELD_LEFT, neigh);
				field_addneigh(neigh, FIELD_RIGHT, field);
			}
		}
		++x;
	}
	fclose(f);
	if (--x > w) w = x;
	field_setdimensions(w, y);
	return this;
}

void
free_map(map_t *this)
{
	free_list(this->fields);
	free(this);
}

void
map_draw(map_t *this)
{
	grBegin();
	grClear();
	list_foreach(this->fields, (void (*)(void *))field_draw);
	grEnd();
}

int
map_isdone(map_t *this)
{
	int isdone = 1;
	field_t *field;
	list_iterator_t *i = new_list_iterator(this->fields);
	while ((field = list_iterator_current(i)) != NULL) {
		int ft = field_gettype(field);
		if ((ft & (FIELD_DEST | FIELD_BOX)) &&
		(ft & (FIELD_DEST | FIELD_BOX)) != (FIELD_DEST | FIELD_BOX)) {
			isdone = 0;
			break;
		}
		list_iterator_next(i);
	}
	free_list_iterator(i);
	return isdone;
}

