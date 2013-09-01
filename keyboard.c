#include "keyboard.h"
#include "menu.h"

void bindGameplayKeystate(App *app){
  Uint8 *keystate;
  keystate = SDL_GetKeyState(NULL);

  player_move(app, &app->game.board.indy.body,
	  keystate[SDLK_w],
	  keystate[SDLK_d],
	  keystate[SDLK_s],
	  keystate[SDLK_a],
	  keystate[SDLK_LCTRL] || keystate[SDLK_LALT]
  );
}

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
		case SDLK_SPACE:
			playerAttack(app, &app->game.board.indy);
			break;
		case SDLK_p:
			playerDie(app, &app->game.board.indy);
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

	if(app->state == STATE_PLAYING){
		bindGameplayKeystate(app);
	}
}

