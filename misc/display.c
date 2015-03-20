#include <SDL/SDL.h>
#include "display.h"

static SDL_Surface *screen = NULL;

int display_frame(char *pixels) {
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *)pixels,
		256, 240, 24, 256*3, 0x0000FF, 0x00FF00, 0xFF0000, 0);

	SDL_BlitSurface(surface, NULL, screen, NULL);
	SDL_Flip(screen);

	return 0;
}

int display_events() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			return 1;	
	}

	return 0;
}

int display_init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(256, 240, 24, SDL_SWSURFACE);
	SDL_Flip(screen);

	return 0;
}

int display_clean() {
	SDL_Quit();

	return 0;
}
