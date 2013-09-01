#include "gameplay.h"

void renderLifeBars(App *app){
	Board *board = &app->game.board;
	SDL_Surface *screen = app->screen;
	renderPlayerLife(screen, board, &board->indy, 0);
	renderPlayerLife(screen, board, &board->allan, 650);
}

void checkPlayerLife(Player *player){
	if(player->life <= 10 && player->body.action == ACTION_DEATH){
		player->life += 0.05;
		if(player->life > 10) player->life = 10;
	}
}

void renderGameplay(App *app){
	renderLifeBars(app);
	renderBody(app, &app->game.board.indy.body, &app->game.board.indy);
	renderBody(app, &app->game.board.allan.body, &app->game.board.allan);

	checkPlayerLife(&app->game.board.indy);
	checkPlayerLife(&app->game.board.allan);
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
