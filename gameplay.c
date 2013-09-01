#include "gameplay.h"

void renderLifeBars(App *app){
	Board *board = &app->game.board;
	SDL_Surface *screen = app->screen;
	renderPlayerLife(screen, board, &board->indy, 0);
	renderPlayerLife(screen, board, &board->allan, 650);
}

void renderGameplay(App *app){
	renderLifeBars(app);
	renderBody(app, &app->game.board.indy.body);
}

void playerAttack(App *app, Player *player){
	Body *body = &player->body;
	body->action = ACTION_ATTACK;
	body->frame = 0;
}
