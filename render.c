#include <SDL_image.h>
#include <math.h>
#include "render.h"

SDL_Color red = {0xAA, 0X55, 0x00};
SDL_Color trueRed = {0XFF, 0x00, 0x00};
SDL_Color white = {0xFF, 0XFF, 0xFF};
SDL_Color green = {0x00, 0XFF, 0x00};
SDL_Color yellow = {0xFF, 0XFF, 0x00};

void renderBody(App *app, Body *body, Player *player){
	if(body->action == ACTION_ATTACK){
		body->frame += 0.3;

		if(body->frame >= body->sprite->frame_count){
			body->action = ACTION_MOVE;
		}
	} else	if(body->action == ACTION_DEATH){
		body->frame += 0.2;

		SDL_Rect src;
		sprite_rotated_rect(
				body->sprite, 
				body->action, 
				(int)body->frame,
				(int)body->angle,
				&src);

		if(body->frame >= body->sprite->frame_count){
			body->frame = 0;
		}

		if(player->life == 10){
			body->action = ACTION_MOVE;
		}

		SDL_BlitSurface(body->sprite->rotated, &src, app->screen, &body->pos);
	
		return;
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
//  app->menu.indiana = IMG_Load("data/goldhead.jpg");
  app->hearts.full = IMG_Load("data/fullheart_small.png");
  app->hearts.onequarter= IMG_Load("data/14heart_small.png");
  app->hearts.twoquarter= IMG_Load("data/24heart_small.png");
  app->hearts.threequarter= IMG_Load("data/34heart_small.png");
  app->hearts.empty = IMG_Load("data/emptyheart_small.png");

  sprite_init(&app->game.indy.sprite, 
	  0, 0, // origin
	  32, 96, 4, // frame size and count
	  "data/indy.png" // source
  );
  app->game.indy.body.sprite = &app->game.indy.sprite;
  app->game.indy.body.pos.x = 200;
  app->game.indy.body.pos.y = 400;

  sprite_init(&app->game.allan.sprite, 
	  0, 0, // origin
	  32, 96, 4, // frame size and count
	  "data/allan.png" // source
  );
  app->game.allan.body.sprite = &app->game.allan.sprite;
  app->game.allan.body.pos.x = 400;
  app->game.allan.body.pos.y = 400;

  sprite_init(&app->zombie, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  "data/zombie.png" // source
  );

  sprite_init(&app->game.head.sprite, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  "data/head.png" // source
  );
  app->game.head.body.sprite = &app->game.head.sprite;
  app->game.head.body.pos.x = 300;
  app->game.head.body.pos.y = 600;
}

void renderTerminate(App *app){
	  SDL_Flip(app->screen);
}

void renderPlayerLife(App *app, SDL_Surface *screen, Player *player, int playerOffset){
	SDL_Color yellow = {0xFF, 0XFF, 0x00};
	int i;
	int ipart = (int) player->life; 
	float fpart = player->life - ipart;

	for(i = 0; i < 10; i++){
		text_write_raw(screen, 35 + playerOffset, 10, player->name, yellow, 20);
		SDL_Rect heartpos= {30 * (i+1) + playerOffset, 40, screen->w, screen->h};

		if(i < player->life){
			if(i < ipart ){
				SDL_BlitSurface(app->hearts.full, NULL, screen, &heartpos);
			} else if(fpart <= 0.25){
				SDL_BlitSurface(app->hearts.onequarter, NULL, screen, &heartpos);
			} else if (fpart <= 0.5){
				SDL_BlitSurface(app->hearts.twoquarter, NULL, screen, &heartpos);
			} else if(fpart <= 0.75){
				SDL_BlitSurface(app->hearts.threequarter, NULL, screen, &heartpos);
			} else {
				SDL_BlitSurface(app->hearts.full, NULL, screen, &heartpos);
			}
		} else {
			SDL_BlitSurface(app->hearts.empty, NULL, screen, &heartpos);
		}
	}
}

