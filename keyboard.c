#include "keyboard.h"

void bindGameplayKeys(App *app, SDLKey *key){
	switch(*key){
		case SDLK_ESCAPE:
			app->state = STATE_EXIT;
			break;
		case SDLK_RETURN:
			app->state = 
	}
}

void bindKeyboard(App *app){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type) {
			case SDL_KEYDOWN:
				if(app->state == STATE_MENU){
					bindMenyKeys(app, &event.key.keysym.sym);
				}
		}
	}
}
