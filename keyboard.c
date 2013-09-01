#include "keyboard.h"
#include "menu.h"

void bindGameplayKeystate(App *app){
  Uint8 *keystate;
  keystate = SDL_GetKeyState(NULL);

  if(keystate[SDLK_SPACE]){
	  playerChargeSpecialAttack(app, &app->game.indy);
  }

  if(keystate[SDLK_RSHIFT]){
	  playerChargeSpecialAttack(app, &app->game.allan);
  }

  movePlayer(app, &app->game.indy,
	  keystate[SDLK_w],
	  keystate[SDLK_d],
	  keystate[SDLK_s],
	  keystate[SDLK_a],
	  0
  );


  movePlayer(app, &app->game.allan,
	  keystate[SDLK_UP],
	  keystate[SDLK_RIGHT],
	  keystate[SDLK_DOWN],
	  keystate[SDLK_LEFT],
	  0
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
		case SDLK_RSHIFT:
		case SDLK_RETURN:
		case SDLK_SPACE:
			if(app->credits){
				app->credits = 0;
			} else {
				chooseMenu(app, menu);
			}
			break;
	}
}

void bindGameplayKeys(App *app, SDLKey *key){
	Menu *menu = &app->menu;
	switch(*key){
		case SDLK_ESCAPE:
			app->state = STATE_PAUSED;
			app->menu.selected = MENU_RESUME;
			break;
		case SDLK_p:
			playerDie(app, &app->game.indy);
			break;
		case SDLK_l:
			playerDie(app, &app->game.allan);
			break;
		case SDLK_0:
			app->debug = (app->debug + 1) % DEBUG_COUNT;
			break;
		case SDLK_SPACE:
		case SDLK_RSHIFT:
			if(app->game.winner){
				app->state = STATE_MENU;
				return;
			}
			break;
	}
}

void bindGameplayKeyUp(App *app, SDLKey *key){
	Menu *menu = &app->menu;
	switch(*key){
		case SDLK_SPACE:
			playerAttack(app, &app->game.indy);
			break;
		case SDLK_RSHIFT:
			if(app->game.winner){
				app->state = STATE_MENU;
				return;
			}
			playerAttack(app, &app->game.allan);
			break;
	}
}

void bindKeyboard(App *app){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type) {
			case SDL_KEYDOWN:
				if(app->state == STATE_MENU || app->state == STATE_PAUSED || app->credits){
					bindMenuKeys(app, &event.key.keysym.sym);
				} else if(app->state == STATE_PLAYING){
					bindGameplayKeys(app, &event.key.keysym.sym);
				}
				break;
			case SDL_KEYUP:
				bindGameplayKeyUp(app, &event.key.keysym.sym);
				break;
		}
	}

	if(app->state == STATE_PLAYING){
		bindGameplayKeystate(app);
	}
}
