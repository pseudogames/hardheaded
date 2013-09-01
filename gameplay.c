#include "gameplay.h"

void renderLifeBars(App *app){
	SDL_Surface *screen = app->screen;
	renderPlayerLife(app, screen, &app->game.indy, 0);
	renderPlayerLife(app, screen, &app->game.allan, 650);
}

void checkPlayerLife(Player *player){
	if(player->life <= 10 && player->body.action == ACTION_DEATH){
		player->life += 0.05;
		if(player->life > 10) player->life = 10;
	}
}

void renderGameplay(App *app){
	renderLifeBars(app);
	renderBody(app, &app->game.indy.body, &app->game.indy);
	renderBody(app, &app->game.allan.body, &app->game.allan);

	checkPlayerLife(&app->game.indy);
	checkPlayerLife(&app->game.allan);
}

void playerAttack(App *app, Player *player){
	Body *body = &player->body;

	if(body->action == ACTION_DEATH) return;
	
	body->action = ACTION_ATTACK;
	body->frame = 0;
}

void playerDie(App *app, Player *player){
	player->life = 0;
	Body *body = &player->body;
	body->action = ACTION_DEATH;
	body->frame = 0;
}
