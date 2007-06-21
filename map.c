/**
 * map.c
 */

#define _map_C	1

#include <malloc.h>
#include <stdio.h>

#include "list.h"

typedef struct {
	int num;
	struct _list_t *fields;
	int notinplace;
	int width, height;
} map_t;

#include "map.h"
#include "gr.h"
#include "player.h"
#include "field.h"
#include "main.h"

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

int
map_load(map_t *this, player_t *player, FILE *f)
{
	int x = 0, y = 0;
	int ch, i;

	list_flush(this->fields);
	this->notinplace = 0;
	this->width = this->height = 0;
	while ((ch = fgetc(f)) != EOF) {
		i = -1;
		switch (ch) {
			case '\n':
				++y;
				if (x > this->width) this->width = x;
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
				++this->notinplace;
				break;
			case 'x':
				i = FIELD_FREE | FIELD_DEST;
				break;
			case '@':
				i = FIELD_FREE | FIELD_DEST | FIELD_BOX;
				break;
			case 'X':
				i = FIELD_FREE | FIELD_DEST | FIELD_PLAYER;
				break;
			default:
				break;
		}
		if (i >= 0) {
			field_t *neigh;
			field_t *field = new_field(x, y, i, this);
			if (field == NULL) {
				return !0;
			}
			list_insert(this->fields, field);
			if (i & FIELD_PLAYER) {
				player_setpos(player, field);
				player_resetcounters(player);
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
	if (--x > this->width) this->width = x;
	this->height = y;
	return 0;
}

map_t *
new_map(int num, player_t *player)
{
	map_t *this;
	char fname[8]; /* "maps/xx" */
	int i;
	FILE *f;

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
	i = map_load(this, player, f);
	fclose(f);
	if (i || this->width < 3 || this->height < 3) {
		free_map(this);
		return NULL;
	}
	field_setdimensions(this->width, this->height);
	return this;
}

void
free_map(map_t *this)
{
	if (this == NULL) {
		return;
	}
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
map_getnum(map_t *this)
{
	return this->num;
}

int
map_count()
{
	static int result = -1;

	if (result < 0) {
		int hi, lo, me, bo;
		char fname[8]; /* "maps/xx" */

		lo = 1;
		hi = 0xff;
		while (hi >= lo) {
			FILE *f;

			me = (lo + hi) / 2;
			snprintf(fname, sizeof(fname), "maps/%02x", me);
			f = fopen(fname, "r");
			bo = (f != NULL);
			if (bo) {
				lo = me + 1;
				fclose(f);
			} else {
				hi = me - 1;
			}
/*			printf("%s %d %d %d %d\n", fname, bo, lo, me, hi);*/
		}
		result = bo ? me : (me - 1);
	}
	return result;
}

void
map_box_arrive(map_t *this)
{
	--this->notinplace;
}

void
map_box_leave(map_t *this)
{
	++this->notinplace;
}

int
map_isdone(const map_t *this)
{
	return !this->notinplace;
}

