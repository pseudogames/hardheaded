#include "gameplay.h"
#include "font.h"

void renderPlayer(SDL_Surface *screen, Board *board, Player *player, int playerOffset){
	SDL_Color yellow = {0xFF, 0XFF, 0x00};
	int i;
	int ipart = (int) player->life; 
	float fpart = player->life - ipart;

	for(i = 1; i <= 10; i++){
		text_write_raw(screen, 35 + playerOffset, 10, player->name, yellow, 20);
		SDL_Rect heartpos= {30 * i + playerOffset, 40, screen->w, screen->h};

		if(i <= player->life){
			if(i < ipart){
				SDL_BlitSurface(board->hearts.full, NULL, screen, &heartpos);
			} else if(fpart == 0.25){
				SDL_BlitSurface(board->hearts.onequarter, NULL, screen, &heartpos);
			} else if (fpart == 0.5){
				SDL_BlitSurface(board->hearts.twoquarter, NULL, screen, &heartpos);
			} else if(fpart == 0.75){
				SDL_BlitSurface(board->hearts.threequarter, NULL, screen, &heartpos);
			} else {
				SDL_BlitSurface(board->hearts.empty, NULL, screen, &heartpos);
			}
		} else {
			SDL_BlitSurface(board->hearts.empty, NULL, screen, &heartpos);
		}
	}
}

void renderLifeBars(App *app){
	Board *board = &app->game.board;
	SDL_Surface *screen = app->screen;
	renderPlayer(screen, board, &board->indy, 0);
	renderPlayer(screen, board, &board->allan, 650);
}

void renderGameplay(App *app){
	renderLifeBars(app);
}

