#include <SDL/SDL.h>
#include <stdbool.h>
#include "display.h"
#include "../cpu/cpu.h"

static SDL_Surface *screen = NULL;

static uint8_t lastWrite;
static int numReads = 0;

static bool a_state = false;
static bool b_state = false;
static bool select_state = false;
static bool start_state = false;
static bool up_state = false;
static bool down_state = false;
static bool left_state = false;
static bool right_state = false;

int display_frame(char *pixels) {
	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *)pixels + 256*3*8*9,
		256*3, 224*3, 24, 256*3*3, 0x0000FF, 0x00FF00, 0xFF0000, 0);

	SDL_BlitSurface(surface, NULL, screen, NULL);
	SDL_Flip(screen);

	return 0;
}

int display_events() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			return 1;	
		} else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_a:
					a_state = true;
					break;
				case SDLK_b:
					b_state = true;
					break;
				case SDLK_RSHIFT:
					select_state = true;
				case SDLK_LSHIFT:
					select_state = true;
					break;
				case SDLK_RETURN:
					start_state = true;
					break;
				case SDLK_UP:
					up_state = true;
					break;
				case SDLK_DOWN:
					down_state = true;
					break;
				case SDLK_LEFT:
					left_state = true;
					break;
				case SDLK_RIGHT:
					right_state = true;
					break;
				case SDLK_ESCAPE:
					cpu_interrupt(RESET);
					break;
				default:
					break;
			}
		} else if (event.type == SDL_KEYUP) {
			switch (event.key.keysym.sym) {
				case SDLK_a:
					a_state = false;
					break;
				case SDLK_b:
					b_state = false;
					break;
				case SDLK_RSHIFT:
					select_state = false;
				case SDLK_LSHIFT:
					select_state = false;
					break;
				case SDLK_RETURN:
					start_state = false;
					break;
				case SDLK_UP:
					up_state = false;
					break;
				case SDLK_DOWN:
					down_state = false;
					break;
				case SDLK_LEFT:
					left_state = false;
					break;
				case SDLK_RIGHT:
					right_state = false;
					break;
				default:
					break;
			}
		}
	}

	return 0;
}

int display_init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(256*3, 224*3, 24, SDL_SWSURFACE);
	SDL_Flip(screen);

	return 0;
}

int display_clean() {
	SDL_Quit();

	return 0;
}

uint8_t controller_read() {
	SDL_Event event;
	uint8_t value;

	// TODO this could potentially skip window closes
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_a:
					a_state = true;
					break;
				case SDLK_b:
					b_state = true;
					break;
				case SDLK_RSHIFT:
					select_state = true;
				case SDLK_LSHIFT:
					select_state = true;
					break;
				case SDLK_RETURN:
					start_state = true;
					break;
				case SDLK_UP:
					up_state = true;
					break;
				case SDLK_DOWN:
					down_state = true;
					break;
				case SDLK_LEFT:
					left_state = true;
					break;
				case SDLK_RIGHT:
					right_state = true;
					break;
				default:
					break;
			}
		} else if (event.type == SDL_KEYUP) {
			switch (event.key.keysym.sym) {
				case SDLK_a:
					a_state = false;
					break;
				case SDLK_b:
					b_state = false;
					break;
				case SDLK_RSHIFT:
					select_state = false;
				case SDLK_LSHIFT:
					select_state = false;
					break;
				case SDLK_RETURN:
					start_state = false;
					break;
				case SDLK_UP:
					up_state = false;
					break;
				case SDLK_DOWN:
					down_state = false;
					break;
				case SDLK_LEFT:
					left_state = false;
					break;
				case SDLK_RIGHT:
					right_state = false;
					break;
				default:
					break;
			}
		}
	}

	switch (numReads) {
		case 0:
			value = a_state;
			break;
		case 1:
			value = b_state;
			break;
		case 2:
			value = select_state;
			break;
		case 3:
			value = start_state;
			break;
		case 4:
			value = up_state;
			break;
		case 5:
			value = down_state;
			break;
		case 6:
			value = left_state;
			break;
		case 7:
			value = right_state;
			break;
		default:
			numReads = -1;
	}
	
	numReads++;

	return value;
}

int controller_write(uint8_t value) {
	if ((lastWrite & 0x01) && !(value & 0x01))
		numReads = 0;

	lastWrite = value;
	return 0;
}
