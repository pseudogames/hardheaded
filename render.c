#include <SDL_image.h>
#include <math.h>
#include "render.h"

SDL_Color red = {0xAA, 0X55, 0x00};
SDL_Color trueRed = {0XFF, 0x00, 0x00};
SDL_Color white = {0xFF, 0XFF, 0xFF};
SDL_Color green = {0x00, 0XFF, 0x00};
SDL_Color yellow = {0xFF, 0XFF, 0x00};

void renderBody(App *app, Body *body){
	if(body->action != ACTION_MOVE){
		body->frame += 0.3;

		if(body->frame >= body->sprite->frame_count){
			body->action = ACTION_MOVE;
		}
	}

	SDL_Rect src;
	sprite_rotated_rect(
			body->sprite, 
			body->action, 
			(int)body->frame,
			(int)body->angle,
			&src);

	SDL_BlitSurface(body->sprite->rotated, &src, app->screen, &body->pos);
}

void renderInit(App *app){
  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE |SDL_DOUBLEBUF) ;
  app->logo = IMG_Load("data/logo.png");
  app->menu.indiana = IMG_Load("data/goldhead.jpg");
  app->game.board.hearts.full = IMG_Load("data/fullheart_small.png");
  app->game.board.hearts.onequarter= IMG_Load("data/14heart_small.png");
  app->game.board.hearts.twoquarter= IMG_Load("data/24heart_small.png");
  app->game.board.hearts.threequarter= IMG_Load("data/34heart_small.png");
  app->game.board.hearts.empty = IMG_Load("data/emptyheart_small.png");

  sprite_init(&app->game.board.indy.sprite, 
	  0, 0, // origin
	  32, 96, 4, // frame size and count
	  "data/indy.png" // source
  );
  app->game.board.indy.body.sprite = &app->game.board.indy.sprite;
  app->game.board.indy.body.pos.x = 100;
  app->game.board.indy.body.pos.y = 600;

  sprite_init(&app->game.board.zombie.sprite, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  "data/zombie.png" // source
  );
  app->game.board.zombie.body.sprite = &app->game.board.zombie.sprite;
  app->game.board.zombie.body.pos.x = 200;
  app->game.board.zombie.body.pos.y = 600;

  sprite_init(&app->game.board.head.sprite, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  "data/head.png" // source
  );
  app->game.board.head.body.sprite = &app->game.board.head.sprite;
  app->game.board.head.body.pos.x = 300;
  app->game.board.head.body.pos.y = 600;

}

void renderTerminate(App *app){
}

void renderPlayerLife(SDL_Surface *screen, Board *board, Player *player, int playerOffset){
	SDL_Color yellow = {0xFF, 0XFF, 0x00};
	int i;
	int ipart = (int) player->life; 
	float fpart = player->life - ipart;

	for(i = 0; i < 10; i++){
		text_write_raw(screen, 35 + playerOffset, 10, player->name, yellow, 20);
		SDL_Rect heartpos= {30 * (i+1) + playerOffset, 40, screen->w, screen->h};

		if(i < player->life){
			if(i < ipart ){
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

