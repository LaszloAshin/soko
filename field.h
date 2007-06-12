/**
 * field.h
 */

#ifndef _field_H
#define _field_H	1

typedef enum {
	FIELD_UP,
	FIELD_RIGHT,
	FIELD_DOWN,
	FIELD_LEFT
} fielddir_t;

typedef enum {
	FIELD_WALL	= 0x00,
	FIELD_FREE	= 0x01,
	FIELD_DEST	= 0x02,
	FIELD_BOX	= 0x04,
	FIELD_PLAYER	= 0x08,
	FIELD_SEEN	= 0x80
} fieldtype_t;

#ifndef _field_C
typedef struct _field_t field_t;
#endif /* _field_C */

field_t *new_field();
void free_field(field_t *this);
void field_addneigh(field_t *this, int dir, field_t *that);
field_t *field_getneigh(const field_t *this, int dir); 
int field_gettype(const field_t *this);
void field_setseen(field_t *this, int bo);
void field_getcoords(const field_t *this, int *x, int *y);
void field_draw(const field_t *this);
field_t *field_playermove(field_t *this, int dir, int *nmoved);
void field_setdimensions(int w, int h);

#endif /* _field_H */

