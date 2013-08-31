#include "keyboard.h"

void bindMenuKeys(App *app, SDLKey *key){
	switch(*key){
		case SDLK_ESCAPE:
			app->state = STATE_EXIT;
			break;
	}
}

void bindKeyboard(App *app){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type) {
			case SDL_KEYDOWN:
					bindMenuKeys(app, &event.key.keysym.sym);
		}
	}
}
