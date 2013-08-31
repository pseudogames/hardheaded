#include "keyboard.h"
#include "menu.h"

void bindMenuKeys(App *app, SDLKey *key){
	Menu *menu = &app->menu;

	int firstMenu = MENU_NEW_GAME;

	if(app->state == STATE_PAUSED){
		firstMenu = MENU_RESUME;
	}

	switch(*key){
		case SDLK_ESCAPE:
			app->state = STATE_EXIT;
			break;
		case SDLK_UP:
		case SDLK_w:
			if(menu->selected > firstMenu){
				menu->selected--;
			}
			break;
		case SDLK_DOWN:
		case SDLK_s:
			if(menu->selected < MENU_COUNT - 1){
				menu->selected++;
			}
			break;
		case SDLK_RETURN:
			chooseMenu(app, menu);
			break;
	}
}

void bindGameplayKeys(App *app, SDLKey *key){
	Menu *menu = &app->menu;
	switch(*key){
		case SDLK_ESCAPE:
			app->state = STATE_PAUSED;
			break;
		case SDLK_i:
			app->game.board.indy.life += 0.25;
			break;
		case SDLK_o:
			app->game.board.indy.life -= 0.25;
			break;
		case SDLK_a:
			app->game.board.allan.life += 0.25;
			break;
		case SDLK_s:
			app->game.board.allan.life -= 0.25;
			break;
	}
}

void bindKeyboard(App *app){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type) {
			case SDL_KEYDOWN:
				if(app->state == STATE_MENU || app->state == STATE_PAUSED){
					bindMenuKeys(app, &event.key.keysym.sym);
				} else if(app->state == STATE_PLAYING){
					bindGameplayKeys(app, &event.key.keysym.sym);
				}
		}
	}
}
