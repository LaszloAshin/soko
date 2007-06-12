#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "player.h"
#include "field.h"
#include "map.h"
#include "gr.h"
#include "menu.h"
#include "inval.h"
#include "main.h"

static map_t *curmap = NULL;
static player_t *player = NULL;

void
postQuit()
{
	SDL_Event ev;

	ev.type = SDL_QUIT;
	SDL_PushEvent(&ev);
}

static void
main_showinfo()
{
	if (curmap == NULL) {
		return;
	}
	grBegin();
	grSetColor(0);
	grBar(8, 250, 231, 311);
	grSetColor(grRGB(0xff, 0xff, 0));
	grRectangle(8, 250, 231, 311);
	grSetColor(grRGB(0xa0, 0xa0, 0xa0));
	grSetPos(16, 254);
	grprintf("Current map: %d", map_getnum(curmap));
	grSetPos(16, 270);
	grprintf("Moves: %d", player_moves(player));
	grSetPos(16, 286);
	grprintf("Pushes: %d", player_pushes(player));
	grEnd();
}

static void (*keyboard)(int key);

static void
main_playerkeyboard(int key)
{
	player_keyboard(player, key);
	if (curmap != NULL && map_isdone(curmap)) {
		if (main_nextmap()) {
			main_switchtogame();
		} else {
			main_switchtomenu();
		}
	}
}

void
main_switchtomenu()
{
	keyboard = menu_keyboard;
	menu_draw();
	main_showinfo();
}

void
main_switchtogame()
{
	if (curmap == NULL) return;
	keyboard = main_playerkeyboard;
	map_draw(curmap);
}

static void
main_mapselected(int allright, int num)
{
	if (!allright || !main_loadmap(num)) {
		main_switchtomenu();
	} else {
		main_switchtogame();
	}
}

void
main_switchtomapsel()
{
	inval_setargs("Select map", 1, map_count(), 0, main_mapselected);
	keyboard = inval_keyboard;
	inval_draw();
}

int
main_loadmap(int num)
{
	map_t *map = new_map(num, player);
	if (map == NULL) return 0;
	free_map(curmap);
	curmap = map;
	return !0;
}

int
main_restartmap()
{
	if (curmap == NULL) return 0;
	return main_loadmap(map_getnum(curmap));
}

int
main_nextmap()
{
	if (curmap == NULL) return 0;
	return main_loadmap(map_getnum(curmap) + 1);
}

int
main_prevmap()
{
	if (curmap == NULL) return 0;
	return main_loadmap(map_getnum(curmap) - 1);
}

static int SDLCALL
EventFilter(const SDL_Event *ev)
{
	return ev->type == SDL_KEYDOWN || ev->type == SDL_QUIT;
}

static void
main_init()
{
	SDL_Surface *sface;

	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
	SDL_WM_SetCaption("soko", "soko");
	sface = SDL_SetVideoMode(240, 320, 16, SDL_HWSURFACE);
	if (sface == NULL) {
		SDL_Quit();
		exit(1);
	}
	if (!grSetSurface(sface)) {
		SDL_Quit();
		exit(1);
	}
	SDL_EnableKeyRepeat(250, 30);
	SDL_EnableUNICODE(1);
	SDL_SetEventFilter(EventFilter);
	player = new_player();
	main_switchtomenu();
}

static void
main_done()
{
	free_map(curmap);
	free_player(player);
	SDL_Quit();
}

static void
main_run()
{
	SDL_Event ev;

	while (SDL_WaitEvent(&ev) == 1 && ev.type != SDL_QUIT) {
		switch (ev.type) {
			case SDL_KEYDOWN:
				keyboard(ev.key.keysym.sym);
				break;
		}
	}
}

int
main()
{
	main_init();
	main_run();
	main_done();
	return 0;
}

