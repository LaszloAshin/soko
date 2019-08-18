/**
 * list.c
 */

#include <malloc.h>
#include <assert.h>
#include <stddef.h>

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
 * Removes all items from the list.
 */
void
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
 * Sets the compare function belongs to the items.
 */
void
list_cmpfunc(list_t *this, int (*cmpfunc)(const void *a, const void *b))
{
	this->cmp = cmpfunc;
}

/**
 * Creates a new empty list.
 * @param free
 *   The functions used to free one list item.
 *   If not NULL, it will be called on disposing an item.
 * @param flags
 *   Finetune the behaviour of the list.
 * @return
 *   Pointer to the brand new list.
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
 * Frees the list by disposing all items.
 */
void
free_list(list_t *this)
{
	list_flush(this);
	free(this);
}

/**
 * Inserts a new item at the end of the list.
 * This may breaks the sorting! If you want to add an item
 * keeping the list sorted, you have to use list_insert.
 * @see list_insert
 * @param data
 *   Pointer to the new item which has to be inserted.
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
 * Inserts a new item as the first item of the list, or
 * if the list is sorted then it inserts it to the appropriate
 * position. In the former case a compare function must be already
 * defined!
 * @param data
 *   Pointer to the new item which has to be inserted.
 */
void
list_insert(list_t *this, void *data)
{
	item_t *i = (item_t *)malloc(sizeof(item_t));
	i->data = data;
	if (this->flags & LIST_SORT) {
		item_t *j, *k = NULL;

		/* 
		 * Find the appropriat position in the sorted list
		 */
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
		/*
		 * If the previous loop haven't found the place, we
		 * need to insert to the last position.
		 */
		if (j == NULL) {
			i->next = NULL;
			*this->last = i;
			this->last = &i->next;
		}
	} else {
		/* 
		 * If the list is not sorted, we just insert to the
		 * first position. We don't forget to update the
		 * last-pointer.
		 */
		if (this->first == NULL) {
			this->last = &i->next;
		}
		i->next = this->first;
		this->first = i;
	}
	++this->nitems;
}

/**
 * Removes the defined items from the list.
 * @param data
 *   Pointer to the item to be removed.
 * @return
 *   Zero if successfuly deleted,
 *   non-zero if not found.
 */
int
list_delete(list_t *this, void *data)
{
	item_t *i, *j;
	int bo = 1;

	/*
	 * Delete if it is in the first position.
	 */
	for (;;) {
		if (this->first == NULL) {
			this->last = &this->first;
			return bo;
		}
		if (this->first->data != data) break;
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
	 * We can only get here if the first item is not NULL and
	 * also not desired to delete.
	 */
	i = this->first;
	while (i->next != NULL) {
		if (i->next->data == data) {
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
	/*
	 * i->next == NULL is true here, so that it's easy to
	 * update the last-pointer.
	 */
	this->last = &i->next;
	return bo;
}

/**
 * Searches an item using the already defined compare function.
 * @param
 *   Item to find. The compare function compares the content of
 *   this one to the content of each items.
 * @return
 *   Pointer to the found item or NULL if not found.
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
 * Removes the first item from the list and gives it back.
 * list_insert and list_delfirst makes the list suitable for
 * make a stack.
 * @see list_insert
 * @return
 *   Pointer to the first item in the list.
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
 * Creates a new iterator over the list.
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
 * Fees up the iterator given.
 */
void
free_list_iterator(list_iterator_t *this)
{
	free(this);
}

/**
 * Returns the current item pointed by the iterator.
 * @return
 *   Pointer to the current item.
 */
void *
list_iterator_current(const list_iterator_t *this)
{
	return (this->current != NULL) ? this->current->data : NULL;
}

/**
 * Moves the iterator to the next item.
 */
void
list_iterator_next(list_iterator_t *this)
{
	if (this->current != NULL) {
		this->current = this->current->next;
	}
}

/**
 * Makes the iterator to point to the first item again.
 */
void
list_iterator_reset(list_iterator_t *this)
{
	this->current = this->list->first;
}

/**
 * Fast iterator without using dynamic memory.
 * This uses a callback function to work on each item.
 * @param cb
 *   A callback function which will get each item as a pointer
 *   as its first argument.
 */
void
list_foreach(const list_t *this, void (*cb)(void *data))
{
	item_t *i;

	for (i = this->first; i != NULL; i = i->next) {
		cb(i->data);
	}
}

