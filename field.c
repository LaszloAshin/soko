/**
 * field.c
 */

#define _field_C	1

static int blocksize = 10;
static int x0, y0;

#include <malloc.h>

#include "gr.h"

typedef struct field_s {
	int x, y;
	int type;
	struct field_s *neigh[4];
	struct _map_t *map;
} field_t;

#include "map.h"
#include "field.h"

field_t *
new_field(int x, int y, int type, map_t *map)
{
	field_t *this;
	int i;

	this = (field_t *)malloc(sizeof(field_t));
	this->x = x;
	this->y = y;
	this->type = type;
	this->map = map;
	for (i = 0; i < 4; ++i) {
		this->neigh[i] = NULL;
	}
	return this;
}

void
free_field(field_t *this)
{
	free(this);
}

void
field_addneigh(field_t *this, int dir, field_t *that)
{
	if ((unsigned)dir > 4) {
		return;
	}
	this->neigh[dir] = that;
}

field_t *
field_getneigh(const field_t *this, int dir)
{
	if ((unsigned)dir > 4) {
		return NULL;
	}
	return this->neigh[dir];
}

int
field_gettype(const field_t *this)
{
	return this->type;
}

void
field_setseen(field_t *this, int bo)
{
	if (bo) {
		this->type |= FIELD_SEEN;
	} else {
		this->type &= ~FIELD_SEEN;
	}
}

void
field_getcoords(const field_t *this, int *x, int *y)
{
	*x = this->x;
	*y = this->y;
}

void
field_draw(const field_t *this)
{
	int x1 = x0 + this->x * blocksize;
	int y1 = y0 + this->y * blocksize;
	int x2 = x1 + blocksize - 1;
	int y2 = y1 + blocksize - 1;

	if (!this->type) {
		grSetColor(grRGB(0x80, 0x80, 0x80));
	} else if (this->type & FIELD_BOX) {
		grSetColor(grRGB(0xff, 0xa0, 0x60));
	} else if (this->type & FIELD_DEST) {
		grSetColor(grRGB(0xff, 0xff, 0xaa));
	} else {
		grSetColor(grRGB(0xff, 0xff, 0xff));
	}
	grBar(x1, y1, x2, y2);
	grSetColor(0);
	if (!this->type) {
		if (this->neigh[FIELD_UP] != NULL &&
		(this->neigh[FIELD_UP]->type & FIELD_FREE)) {
			grLine(x1, y1, x2, y1);
		}
		if (this->neigh[FIELD_RIGHT] != NULL &&
		(this->neigh[FIELD_RIGHT]->type & FIELD_FREE)) {
			grLine(x2, y1, x2, y2);
		}
		if (this->neigh[FIELD_DOWN] != NULL &&
		(this->neigh[FIELD_DOWN]->type & FIELD_FREE)) {
			grLine(x1, y2, x2, y2);
		}
		if (this->neigh[FIELD_LEFT] != NULL &&
		(this->neigh[FIELD_LEFT]->type & FIELD_FREE)) {
			grLine(x1, y1, x1, y2);
		}
	} else if (this->type & FIELD_BOX) {
		grRectangle(x1, y1, x2, y2);
	} else if (this->type & FIELD_PLAYER) {
		grSetColor(grRGB(0, 0xa0, 0));
		grBar(x1 + 1, y1 + 1, x2 - 1, y2 - 1);
		grSetColor(0);
		grRectangle(x1 + 1, y1 + 1, x2 - 1, y2 - 1);
	}
}

field_t *
field_playermove(field_t *this, int dir, int *nmoved)
{
	field_t *neigh1;

	if (nmoved != NULL) *nmoved = 0;
	neigh1 = field_getneigh(this, dir);
	if (neigh1 == NULL) return NULL;
	if (!(neigh1->type & FIELD_FREE)) return NULL;
	if (neigh1->type & FIELD_BOX) {
		field_t *neigh2 = field_getneigh(neigh1, dir);
		if (neigh2 == NULL) return NULL;
		if (!(neigh2->type & FIELD_FREE)) return NULL;
		if (neigh2->type & FIELD_BOX) return NULL;
		neigh1->type &= ~FIELD_BOX;
		neigh2->type |= FIELD_BOX;
		if (nmoved != NULL) ++(*nmoved);
		if (!(neigh1->type & FIELD_DEST)) {
			if (neigh2->type & FIELD_DEST) {
				map_box_arrive(this->map);
			}
		} else {
			if (!(neigh2->type & FIELD_DEST)) {
				map_box_leave(this->map);
			}
		}
		grBegin();
		field_draw(neigh2);
	} else {
		grBegin();
	}
	this->type &= ~FIELD_PLAYER;
	neigh1->type |= FIELD_PLAYER;
	if (nmoved != NULL) ++(*nmoved);
	field_draw(this);
	field_draw(neigh1);
	grEnd();
	return neigh1;
}

void
field_setdimensions(int w, int h)
{
	int sw, sh, x, y;

	grGetScreenDimensions(&sw, &sh);
	x = sw / w;
	y = sh / h;
	if (y < x) x = y;
	blocksize = x;
	x0 = (sw - w * blocksize) / 2;
	y0 = (sh - h * blocksize) / 2;
}

