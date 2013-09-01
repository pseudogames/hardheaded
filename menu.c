#include "menu.h"

void resetApp(App *app)
{
	app->game.total_kill_count= 0;
	app->game.winner = NULL;
	app->game.board.zombie_memory1 = 0;
	app->game.board.zombie_memory2 = 0;

	app->game.indy.name = "Mr. Indy J.";
	app->game.indy.body.life = 10;
	app->game.indy.body.score = 0;
	app->game.indy.body.ang_vel = 0.25;
	app->game.indy.body.max_vel = 4;
	app->game.indy.special_attack = 0;
	app->game.indy.grabbing = 0;

	app->game.allan.name = "Mr. Allan Q.";
	app->game.allan.body.life = 10;
	app->game.allan.body.score = 0;
	app->game.allan.body.ang_vel = 0.25;
	app->game.allan.body.max_vel = 4;
	app->game.allan.special_attack = 0;
	app->game.allan.grabbing = 0;

	app->game.head.body.life = 100;
	app->game.head.body.score = 0;
	app->game.head.body.ang_vel = 0.25;
	app->game.head.body.max_vel= 4;

	memset(&app->game.board.enemies, 0 , sizeof(app->game.board.enemies));
	gameInit(app);
	moveInit(app);
	setWave(app, 0);
}

void chooseMenu(App *app, Menu *menu){
	switch(menu->selected){
		case MENU_NEW_GAME:
			resetApp(app);
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
