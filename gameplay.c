#include "gameplay.h"

void renderLifeBars(App *app){
	SDL_Surface *screen = app->screen;
	Board *board = &app->game.board;

	renderPlayerSpecialBar(screen, board, &app->game.indy, 0);
	renderPlayerSpecialBar(screen, board, &app->game.allan, 650);

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

void playerChargeSpecialAttack(App *app, Player *player){
	if(player->special_attack < 100) {
		player->special_attack += 1;
	}
}

void playerAttack(App *app, Player *player){
	Body *body = &player->body;

	if(body->action == ACTION_DEATH) return;

	if(player->special_attack < 100) {
		body->action = ACTION_ATTACK;
		body->frame = 0;
	}

	player->special_attack = 0;
}

void playerDie(App *app, Player *player){
	player->life = 0;
	Body *body = &player->body;
	body->action = ACTION_DEATH;
	body->frame = 0;
}
