/**
 * list.h
 */

#ifndef _LIST_H
#define _LIST_H	1

#ifndef _LIST_C
typedef struct _list_t list_t;
typedef struct _list_iterator_t list_iterator_t;
#endif /* _LIST_C */

#define LIST_SORT	0x01

void list_flush(list_t *this);
list_t *new_list(void (*freefunc)(void*), int flags);
void list_cmpfunc(list_t *this, int (*cmpfunc)(const void *a, const void *b));
void free_list(list_t *this);
void list_add(list_t *this, void *data);
void list_insert(list_t *this, void *data);
int list_delete(list_t *this, void *data);
void *list_search(list_t *this, const void *data);
void *list_delfirst(list_t *this);

list_iterator_t *new_list_iterator(list_t *l);
void free_list_iterator(list_iterator_t *this);
void *list_iterator_current(const list_iterator_t *this);
void list_iterator_next(list_iterator_t *this);
void list_iterator_reset(list_iterator_t *this);
void list_foreach(const list_t *l, void (*cb)(void *data));

#endif /* _LIST_H */

