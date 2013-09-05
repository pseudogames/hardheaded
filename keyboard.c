#include "keyboard.h"
#include "menu.h"
#include "config.h"

void bindGameplayKeystate(App *app){
  Uint8 *keystate;
  keystate = SDL_GetKeyState(NULL);

  if(keystate[keybind.player1.attack]){
	  playerChargeSpecialAttack(app, &app->game.indy);
  }

  if(keystate[keybind.player2.attack]){
	  playerChargeSpecialAttack(app, &app->game.allan);
  }

  movePlayer(app, &app->game.indy,
	  keystate[keybind.player1.up],
	  keystate[keybind.player1.right],
	  keystate[keybind.player1.down],
	  keystate[keybind.player1.left],
	  0
  );


  movePlayer(app, &app->game.allan,
	  keystate[keybind.player2.up],
	  keystate[keybind.player2.right],
	  keystate[keybind.player2.down],
	  keystate[keybind.player2.left],
	  0
  );
}

void bindMenuKeys(App *app, SDLKey *key){
	Menu *menu = &app->menu;

	int firstMenu = MENU_NEW_GAME;

	if(app->state == STATE_PAUSED){
		firstMenu = MENU_RESUME;
	}

	if( *key == keybind.player1.up || *key == keybind.player2.up || *key == SDLK_UP) {
		if(menu->selected > firstMenu){
			menu->selected--;
		}
	}

	if( *key == keybind.player1.down || *key == keybind.player2.down || *key == SDLK_DOWN) {
		if(menu->selected < MENU_COUNT - 1){
			menu->selected++;
		}
	}

	if( *key == keybind.player1.attack || *key == keybind.player2.attack ||
		*key == SDLK_RETURN || *key == SDLK_SPACE ) {
		if(app->credits == 4){
			app->credits = 0;
		}  else if (app->credits == 0) {
			chooseMenu(app, menu);
		} else {
			app->credits++;
		}
	}
}

void bindGameplayKeys(App *app, SDLKey *key){
	Menu *menu = &app->menu;
	if( *key == keybind.player1.start || *key == keybind.player2.start || *key == SDLK_ESCAPE) {
			if(app->game.winner || app->game.head.body.life <= 0){
				app->state = STATE_MENU;
			} else {
				app->state = STATE_PAUSED;
				app->menu.selected = MENU_RESUME;
			}
	}

#if 0
	if( *key == SDLK_0) {
			app->debug = (app->debug + 1) % DEBUG_COUNT;
			break;
	}
#endif
}

void bindGameplayKeyUp(App *app, SDLKey *key){
	Menu *menu = &app->menu;
	if(*key == keybind.player1.attack) {
		playerAttack(app, &app->game.indy);
	}
	if(*key == keybind.player2.attack) {
		playerAttack(app, &app->game.allan);
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
