/**
 * list.c
 */

#include <malloc.h>
#include <assert.h>

typedef struct item_s {
	void *data;
	struct item_s *next;
} item_t;

typedef struct {
	item_t *first;
	item_t **last;
	int nitems;
	int (*cmp)(const void *a, const void *b);
	int flags;
	void (*free)(void *);
} list_t;

typedef struct {
	list_t *list;
	item_t *current;
} list_iterator_t;

#define _LIST_C 1
#include "list.h"

/**
 * Kiuriti a listat.
 */
static void
list_flush(list_t *this)
{
	while (this->first != NULL) {
		item_t *p = this->first;
		this->first = p->next;
		if (this->free != NULL) {
			this->free(p->data);
		}
		free(p);
	}
	this->first = NULL;
	this->last = &this->first;
	this->nitems = 0;
}

/**
 * Beallitja a listahoz tartozo osszehasonlito fuggvenyt.
 */
void
list_cmpfunc(list_t *this, int (*cmpfunc)(const void *a, const void *b))
{
	this->cmp = cmpfunc;
}

/**
 * Uj listat hoz letre.
 */
list_t *
new_list(void (*free)(void *), int flags)
{
	list_t *this = (list_t *)malloc(sizeof(list_t));
	this->first = NULL;
	this->last = &this->first;
	this->nitems = 0;
	this->cmp = NULL;
	this->flags = flags;
	this->free = free;
	return this;
}

/**
 * Felszabaditja a listat.
 */
void
free_list(list_t *this)
{
	list_flush(this);
	free(this);
}

/**
 * Beszur egy uj elemet a lista vegere.
 * Ha van rendezes, akkor ez elrontja!
 */
void
list_add(list_t *this, void *data)
{
	item_t *i = (item_t *)malloc(sizeof(item_t));
	i->data = data;
	i->next = NULL;
	*this->last = i;
	this->last = &i->next;
	++this->nitems;
}

/**
 * Beszur egy uj elemet a lista elejere, vagy
 * ha be van allitva a rendezes, akkor a megfelelo helyre.
 * Utobbi esetben kell hogy legyen osszehasonlito fuggveny beallitva!
 */
void
list_insert(list_t *this, void *data)
{
	item_t *i = (item_t *)malloc(sizeof(item_t));
	i->data = data;
	if (this->flags & LIST_SORT) {
		item_t *j, *k = NULL;

		for (j = this->first; j != NULL; j = j->next) {
			if (this->cmp(j->data, data) >= 0) {
				i->next = j;
				if (k != NULL) {
					k->next = i;
				} else {
					this->first = i;
				}
				break;
			}
			k = j;
		}
		if (j == NULL) {
			i->next = NULL;
			*this->last = i;
			this->last = &i->next;
		}
	} else {
		i->next = this->first;
		this->first = i;
	}
	++this->nitems;
}

/**
 * Torli a megadott tartalmu elemeket a listabol.
 * Hivasa elott meg kell adni egy osszehasonlito fuggvenyt!
 */
int
list_delete(list_t *this, void *data)
{
	item_t *i, *j;
	int bo = 1;

	/* Ha az elso helyen all akkor toroljuk. */
	for (;;) {
		if (this->first == NULL) return bo;
		if (this->cmp(this->first->data, data)) break;
		j = this->first;
		this->first = this->first->next;
		if (this->free != NULL) {
			this->free(j->data);
		}
		free(j);
		--this->nitems;
		bo = 0;
	}
	/*
	 * Ide csak akkor juthat ha az elso elem nem NULL es
	 * nem is torlendo.
	 */
	i = this->first;
	while (i->next != NULL) {
		if (!this->cmp(i->next->data, data)) {
			j = i->next;
			i->next = i->next->next;
			if (this->free != NULL) {
				this->free(j->data);
			}
			free(j);
			--this->nitems;
			bo = 0;
		} else {
			i = i->next;
		}
	}
	return bo;
}

/**
 * Megkeres egy elemet es visszater a cimevel.
 * A hivasa elott meg kell adni egy osszehasonlito fuggvenyt!
 */
void *
list_search(list_t *this, const void *data)
{
	item_t *i;

	assert(this->cmp != NULL);
	for (i = this->first; i != NULL; i = i->next) {
		if (!this->cmp(i->data, data)) return i->data;
	}
	return NULL;
}

/**
 * Torli a listabol es visszaadja a legelso elemet.
 */
void *
list_delfirst(list_t *this)
{
	item_t *i = this->first;
	if (i == NULL) return NULL;
	this->first = i->next;
	return i->data;
}

/**
 * Uj iteratort hoz letre a listan.
 */
list_iterator_t *
new_list_iterator(list_t *l)
{
	list_iterator_t *this =
		(list_iterator_t *)malloc(sizeof(list_iterator_t));
	this->list = l;
	this->current = l->first;
	return this;
}

/**
 * Felszabaditja az iteratort.
 */
void
free_list_iterator(list_iterator_t *this)
{
	free(this);
}

/**
 * Az iterator altal mutatott elemet kerdezi le.
 */
void *
list_iterator_current(const list_iterator_t *this)
{
	return (this->current != NULL) ? this->current->data : NULL;
}

/**
 * Elore lepteti az iteratort.
 */
void
list_iterator_next(list_iterator_t *this)
{
	if (this->current != NULL) {
		this->current = this->current->next;
	}
}

/**
 * A lista elejere allitja az iteratort.
 */
void
list_iterator_reset(list_iterator_t *this)
{
	this->current = this->list->first;
}

/**
 * Gyors bejaro callback megoldassal.
 */
void
list_foreach(const list_t *this, void (*cb)(void *data))
{
	item_t *i;

	for (i = this->first; i != NULL; i = i->next) {
		cb(i->data);
	}
}

