#include <stdio.h>
#include <SDL/SDL.h>

#include "player.h"
#include "field.h"
#include "map.h"
#include "gr.h"

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
edKeyboard(int key)
{
	int bo = 0;
	switch (key) {
		case SDLK_UP:
			bo = player_move(player, FIELD_UP);
			break;
		case SDLK_RIGHT:
			bo = player_move(player, FIELD_RIGHT);
			break;
		case SDLK_DOWN:
			bo = player_move(player, FIELD_DOWN);
			break;
		case SDLK_LEFT:
			bo = player_move(player, FIELD_LEFT);
			break;
		case SDLK_ESCAPE:
			postQuit();
			break;
		case SDLK_RETURN:
			break;
	}
	if (bo) bo = map_isdone(curmap);
	if (bo) {
		printf("map is done!\n");
	}
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
	curmap = new_map(1, player);
	map_draw(curmap);
	while (!SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_QUITMASK)) {
		SDL_Event ev;

		SDL_PumpEvents();
		if (SDL_PeepEvents(&ev, 1, SDL_GETEVENT, SDL_KEYDOWNMASK) > 0) {
			edKeyboard(ev.key.keysym.sym);
		}
		SDL_Delay(5);
	}
	free_map(curmap);
	free_player(player);
	SDL_Quit();
	return 0;
}

