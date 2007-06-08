/**
 * menu.c
 */

#define _menu_C	1

#include <SDL/SDL_keysym.h>

#include "menu.h"
#include "gr.h"
#include "main.h"

static void
menu_newgame()
{
	if (main_loadmap(1)) {
		main_switchtogame();
	}
}

static void
menu_nextmap()
{
	if (main_nextmap()) {
		main_switchtogame();
	}
}

static void
menu_prevmap()
{
	if (main_prevmap()) {
		main_switchtogame();
	}
}

static struct {
	const char *name;
	void (*action)(void);
} items[] = {
	{ "New game", menu_newgame },
	{ "Next map", menu_nextmap },
	{ "Previous map", menu_prevmap },
	{ "Exit game", postQuit },
	{ NULL, NULL }
};

static int curitem = 0;

static void
menu_drawitem(int i)
{
	grSetColor(0xffff);
	grSetPos(16, i * 16);
	grprintf("%s", items[i].name);
	if (i == curitem) {
		grSetColor(grRGB(0xff, 0xff, 0));
	} else {
		grSetColor(0);
	}
	grOctagon(8, i * 16 + 10, 4);
}

void
menu_keyboard(int key)
{
	int oci = curitem;

	switch (key) {
		case SDLK_UP:
			--curitem;
			break;
		case SDLK_DOWN:
			++curitem;
			break;
		case SDLK_RETURN:
			if (items[curitem].action != NULL) {
				items[curitem].action();
			}
			break;
		case SDLK_ESCAPE:
			main_switchtogame();
			break;
	}
	if (curitem < 0) {
		curitem = 0;
	} else if (items[curitem].name == NULL) {
		--curitem;
	}
	if (curitem != oci) {
		grBegin();
		menu_drawitem(oci);
		menu_drawitem(curitem);
		grEnd();
	}
}

void
menu_draw()
{
	int i;

	grBegin();
	grClear();
	for (i = 0; items[i].name != NULL; ++i) {
		menu_drawitem(i);
	}
	grEnd();
}

