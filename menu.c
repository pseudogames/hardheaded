#include "menu.h"

void chooseMenu(App *app, Menu *menu){
	switch(menu->selected){
		case MENU_NEW_GAME:
			app->state = STATE_PLAYING;
			break;
		case MENU_RESUME:
			app->state = STATE_PLAYING;
			break;
		case MENU_EXIT:
			app->state = STATE_EXIT;
			break;
	}
}

void renderMenu(App *app){
	Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
	SDL_FillRect(app->screen, NULL , color);
	Menu *menu = &app->menu;
	SDL_Surface *screen = app->screen;

	SDL_Rect highlightChar = {-700, -450, screen->w, screen->h};
	SDL_BlitSurface(menu->indiana, &highlightChar, screen, NULL);

	int resumePadding = 0;

	text_write_raw(screen, 200, 50, "HardHeaded", red, 96);

	if(app->state == STATE_PAUSED){
		text_write(screen, 100, 200 + resumePadding, "resume", menu->selected == MENU_RESUME);
	}
	text_write(screen, 100, 350 + resumePadding, "new game", menu->selected == MENU_NEW_GAME);
	text_write(screen, 100, 550 + resumePadding, "exit", menu->selected == MENU_EXIT);
}
