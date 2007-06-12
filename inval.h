/**
 * inval.h
 */

#ifndef _inval_H
#define _inval_H	1

void inval_keyboard(int key);
void inval_draw();
void inval_setargs(const char *msg, int lo, int hi, int def, void (*setval)(int, int));

#endif /* _inval_H */

