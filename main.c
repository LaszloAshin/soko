#include <stdio.h>
#include <SDL/SDL.h>

#include "player.h"
#include "field.h"
#include "map.h"
#include "gr.h"
#include "menu.h"

static map_t *curmap = NULL;
static player_t *player = NULL;

void
postQuit()
{
	SDL_Event ev;

	ev.type = SDL_QUIT;
	SDL_PushEvent(&ev);
}

static void (*keyboard)(int key);

static void
main_playerkeyboard(int key)
{
	player_keyboard(player, curmap, key);
}

void
main_switchtomenu()
{
	keyboard = menu_keyboard;
	menu_draw();
}

void
main_switchtogame()
{
	if (curmap == NULL) return;
	keyboard = main_playerkeyboard;
	map_draw(curmap);
}

int
main_loadmap(int num)
{
	free_map(curmap);
	curmap = new_map(num, player);
	return curmap != NULL;
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

int
main()
{
	SDL_Event ev;
	SDL_Surface *sface;

	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
	SDL_WM_SetCaption("soko", "soko");
	sface = SDL_SetVideoMode(240, 320, 16, SDL_HWSURFACE);
	if (sface == NULL) {
		SDL_Quit();
		return 1;
	}
	if (!grSetSurface(sface)) {
		return 1;
	}
	SDL_EnableKeyRepeat(250, 30);
	SDL_EnableUNICODE(1);
	SDL_SetEventFilter(EventFilter);
	player = new_player();
	main_switchtomenu();
	while (!SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_QUITMASK)) {
		SDL_Event ev;

		SDL_PumpEvents();
		if (SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_KEYDOWNMASK) > 0) {
			keyboard(ev.key.keysym.sym);
		}
		SDL_Delay(5);
	}
	free_map(curmap);
	free_player(player);
	SDL_Quit();
	return 0;
}

