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
