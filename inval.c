/**
 * inval.c
 */

#define _inval_C	1

#include <malloc.h>
#include <stdlib.h>
#include <SDL/SDL_keysym.h>

static struct {
	const char *msg;
	int lo, hi, cur;
	void (*setval)(int, int);
} inval;

#include "inval.h"
#include "gr.h"

static void inval_update();

void
inval_keyboard(int key)
{
	switch (key) {
		case SDLK_UP:
			inval.lo = inval.cur + 1;
			inval_update();
			if (inval.hi <= inval.lo) {
				goto ready;
			}
			break;
		case SDLK_DOWN:
			inval.hi = inval.cur - 1;
			inval_update();
			if (inval.hi <= inval.lo) {
				goto ready;
			}
			break;
		ready:
		case SDLK_RIGHT:
		case SDLK_RETURN:
			inval.setval(!0, inval.cur);
			break;
		case SDLK_ESCAPE:
			inval.setval(0, 0);
			break;
	}
}

static void
inval_update()
{
	inval.cur = (inval.lo + inval.hi) / 2;
	grBegin();
	grSetColor(0);
	grBar(0, 48, 240, 63);
	grSetColor(grRGB(0xff, 0xff, 0));
	grSetPos(0, 48);
	grprintf("%d?", inval.cur);
	grEnd();
}

void
inval_draw()
{
	grBegin();
	grClear();
	grSetColor(grRGB(0xff, 0xff, 0xff));
	grSetPos(0, 0);
	grprintf("%s", inval.msg);
	grSetPos(0, 16);
	grprintf("Choose a number between");
	grSetPos(0, 32);
	grprintf("%d and %d.", inval.lo, inval.hi);
	grSetPos(0, 64);
	grprintf("(UP: grater, DOWN: lower)");
	grSetColor(grRGB(0xff, 0xff, 0));
	grSetPos(0, 48);
	grprintf("%d?", inval.cur);
	grEnd();
}

void
inval_setargs(const char *msg, int lo, int hi, int def, void (*setval)(int, int))
{
	if (lo > hi) {
		return;
	}
	if (def < lo || def > hi) {
		def = (rand() % (hi - lo + 1)) + lo;
	}
	inval.msg = msg;
	inval.lo = lo;
	inval.hi = hi;
	inval.cur = def;
	inval.setval = setval;
}

