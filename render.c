#include <SDL_image.h>
#include <math.h>
#include "render.h"

SDL_Color black = {0, 0, 0};
SDL_Color red = {0xAA, 0X55, 0x00};
SDL_Color trueRed = {0XFF, 0x00, 0x00};
SDL_Color white = {0xFF, 0XFF, 0xFF};
SDL_Color green = {0x00, 0XFF, 0x00};
SDL_Color yellow = {0xFF, 0XFF, 0x00};

void renderDebug(App *app)
{
	int color;
	int *map = NULL;
	switch(app->debug) {
		case DEBUG_WALL: // black
			color = SDL_MapRGBA(app->screen->format, 0x00,0x00,0x00,0xff );
			map = (int *)app->game.board.wall;
			break;
		case DEBUG_AIR: // green
			color = SDL_MapRGBA(app->screen->format, 0x00,0xff,0x00,0xff );
			map = (int *)app->game.board.air;
			break;
		case DEBUG_AI: // white
			color = SDL_MapRGBA(app->screen->format, 0xff,0xff,0xff,0xff );
			map = (int *)walkability;
			break;
		case DEBUG_ENEMY: // red
			color = SDL_MapRGBA(app->screen->format, 0xff,0x00,0x00,0xff );
			map = (int *)app->game.board.spawn_map;
			break;
		case DEBUG_MOVE: // blue
			color = SDL_MapRGBA(app->screen->format, 0x00,0x00,0xff,0xff );
			map = (int *)app->game.board.crowd;
			break;
		case DEBUG_SHOT: // yellow
			color = SDL_MapRGBA(app->screen->format, 0xff,0xff,0x00,0xff );
			map = (int *)app->game.board.hittable;
			break;
		case DEBUG_DEATH1: // dark red
			color = SDL_MapRGBA(app->screen->format, 0x80,0x00,0x00,0xff );
			map = (int *)app->game.board.death1;
			break;
		case DEBUG_DEATH2: // dark purple
			color = SDL_MapRGBA(app->screen->format, 0x80,0x00,0x80,0xff );
			map = (int *)app->game.board.death2;
			break;
	}

	if(map){
		int x,y;
		for (x=0; x < mapWidth;x++) {
			for (y=0; y < mapHeight;y++) {
				if(map[x*mapHeight+y]) {
					SDL_Rect rect = { x*tileSize, y*tileSize, tileSize, tileSize };
					SDL_FillRect(app->screen, &rect , color);
				}
			}
		}

		color = SDL_MapRGBA(app->screen->format, 0xff,0x00,0x00,0xff );
		int i;
		for(i=0; i < ENEMY_COUNT; i++) 
		{
			if(app->game.board.enemies[i].alive)
			{
				extern int pathBank [numberPeople+1][maxPathLength*2];
				int p = app->game.board.enemies[i].pathfinder;
				int s = 2;
				int n = tileSize/s;
				int x1 = i % n;
				int y1 = (i / n) % n;
				int j;
				for(j=0; j<pathLength[p]; j++) {
					int x = pathBank[p][j*2+0];
					int y = pathBank[p][j*2+1];
					SDL_Rect rect = { x*tileSize+x1*s, y*tileSize+y1*s, s, s };
					SDL_FillRect(app->screen, &rect , color);
				}
			}
		}

		SDL_Rect rect = { 250, 20, 0, 0 };
		SDL_BlitSurface(app->game.board.hit, NULL, app->screen, &rect);

	}
}
void renderPlayer(App *app, Player *player){
	Body *body = &player->body;
	if(body->action == ACTION_ATTACK){
		body->frame += 0.3;

		if(body->frame >= body->sprite->frame_count){
			body->action = ACTION_MOVE;
		}
	} else if(body->action == ACTION_DEATH){
		body->frame += 0.2;

		if(body->frame >= body->sprite->frame_count){
			body->frame = 0;
		}

		if(player->body.life == 10){
			body->action = ACTION_MOVE;
		}
	}

	renderBody(app, body);

	if(player->power_body.action == ACTION_DEATH){
		player->power_body.frame += 0.1;

		renderBody(app, &player->power_body);

		if(body->frame >= body->sprite->frame_count){
			player->power_body.action = ACTION_MOVE;
			body->frame = 0;
		}

		return;
	}

	if(player->grabbing == 0 
		&& player->power_body.action == ACTION_ATTACK 
		|| player->special_attack > 7){

		player->power_body.frame += player->special_attack / 100.;
		player->power_body.vel = 5;


		if(player->power_body.action == ACTION_ATTACK && player->special_attack < 98){
			float a = player->power_body.angle * M_PI / 180;
			float dx = cos(a) * 39;
			float dy = sin(a) * 39;
			float tx = player->power_body.pos.x + dx;
			float ty = player->power_body.pos.y - dy;

			player->power_body.frame ++;

			shoot(app, &player->power_body, 100);
			shoot(app, &player->power_body, 100);
			shoot(app, &player->power_body, 100);

			player->body.life += player->power_body.life;
			player->power_body.life = 0;

			if(!is_air(&app->game, &player->power_body, (int)tx, (int)ty)){
				player->power_body.action = ACTION_DEATH;
				player->power_body.frame = 0;
			}

			player->power_body.pos.x = tx;
			player->power_body.pos.y = ty;
		} else {
			float a = player->body.angle * M_PI / 180;
			float dx = cos(a) * tileSize/2;
			float dy = sin(a) * tileSize/2;
			float tx = body->pos.x + dx;
			float ty = body->pos.y - dy;

			player->power_body.angle = body->angle;
			player->power_body.pos.x = tx;
			player->power_body.pos.y = ty;
		}

		renderBody(app, &player->power_body);
	}
}

void renderBody(App *app, Body *body){
	SDL_Rect src;
	sprite_rotated_rect(
			body->sprite, 
			body->action, 
			(int)body->frame,
			(int)body->angle,
			&src);

	SDL_Rect dest = {
		body->pos.x - body->sprite->rotated_frame_size.x/2,
		body->pos.y - body->sprite->rotated_frame_size.y/2,
		0, 0
	};

	SDL_BlitSurface(body->sprite->rotated, &src, app->screen, &dest);
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
  app->special_bar = IMG_Load("data/chargebarr.png");

  sprite_init(&app->game.indy.sprite, 
	  0, 0, // origin
	  32, 96, 4, // frame size and count
	  "data/indy.png" // source
  );
  app->game.indy.body.sprite = &app->game.indy.sprite;

  sprite_init(&app->game.allan.sprite, 
	  0, 0, // origin
	  32, 96, 4, // frame size and count
	  "data/allan.png" // source
  );
  app->game.allan.body.sprite = &app->game.allan.sprite;

  app->enemy_frame_count = 3;
  app->enemy_variation_count = 2;
  sprite_init(&app->zombie, 
	  0, 0, // origin
	  32, 32, app->enemy_frame_count * app->enemy_variation_count, // frame size and count
	  "data/zombie.png" // source
  );

  sprite_init(&app->game.head.sprite, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  "data/head.png" // source
  );

  app->game.head.body.sprite = &app->game.head.sprite;

  sprite_init(&app->game.allan.power, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  "data/power.png" // source
  );

  app->game.allan.power_body.sprite = &app->game.allan.power;


  sprite_init(&app->game.indy.power, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  "data/power.png" // source
  );

  app->game.indy.power_body.sprite = &app->game.indy.power;
}

void renderTerminate(App *app){
}

void renderPlayerLife(App *app, SDL_Surface *screen, Player *player, int playerOffset){
	SDL_Color yellow = {0xff, 0Xe5, 0xd5};
	int i;
	int ipart = (int) player->body.life; 
	float fpart = player->body.life - ipart;

	for(i = 0; i < 10; i++){
		text_write_raw(screen, 35 + playerOffset, 10, player->name, yellow, 20);
		SDL_Rect heartpos= {30 * (i+1) + playerOffset, 40, screen->w, screen->h};

		if(i < player->body.life){
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

void renderPlayerSpecialBar(App *app, SDL_Surface *screen, Player *player, int playerOffset){
	SDL_Rect pos= {35 + playerOffset, 70, screen->w, screen->h};
	SDL_BlitSurface(app->special_bar, NULL, screen, &pos);

	int width = player->special_attack * 3;
	int max_width = 294;
	if(width > max_width) width = max_width;
	if(width < 7) width = 0;

	Uint32 color = SDL_MapRGB(screen->format, 0xFF, 0xFF,0x00 );

	SDL_Rect rect = { 35 + playerOffset + 3, 73, width, 10};
	SDL_FillRect(screen, &rect, color);
}

void renderSpawnCountdown(App *app){
	SDL_Color yellow = {0xFF, 0XFF, 0xFF};
	char msg[256];
	sprintf(msg, "more zombies in %d", (app->game.next_wave - SDL_GetTicks()) / 1000);

	text_write_raw(app->screen, 400 , 10, msg, yellow, 20);
}

void renderHead(App *app){
	Body *body = &app->game.head.body;
	body->frame += 0.3;

	if(body->frame >= body->sprite->frame_count){
		body->frame = 0;
	}

	renderBody(app, body);

	
	if(body->life < HEAD_HEALTH) {
		Uint32 color = SDL_MapRGBA(app->screen->format, 0x00,0x00,0x00,0xff );
		SDL_Rect rect = { body->pos.x-tileSize/2, body->pos.y-tileSize/2, tileSize, 2 };
		SDL_FillRect(app->screen, &rect , color);

		color = SDL_MapRGBA(app->screen->format, 0xff,0x00,0x00,0xff );
		rect.w = rect.w * ( body->life / HEAD_HEALTH);
		SDL_FillRect(app->screen, &rect , color);
	}
}

void renderWinner(App *app){
	Player *winner = app->game.winner;
	char msg[256];
	sprintf(msg, "The winner is: %s", winner->name);
	text_write_raw(app->screen, 20 , 330, msg, white, 60);
}

void renderGameOver(App *app){
	text_write_raw(app->screen, 20 , 330, "Draw. Game Over.", white, 60);
}
void renderHints(App *app){
	Body *head = &app->game.head.body;
	Player *indy = &app->game.indy;
	Player *allan = &app->game.allan;

	if((SDL_GetTicks()/500) % 2 == 0){

		if(indy->grabbing == 0 && allan->grabbing == 0){
			char *msg = "GRAB";
			if(indy->body.action == ACTION_DEATH || indy->body.action == ACTION_DEATH) msg = "GUARD";
			text_write_raw_ttf(app->screen, head->pos.x , head->pos.y - tileSize/2 - 60, "I", trueRed, 60, "data/arrows.ttf");
			text_write_raw_ttf(app->screen, head->pos.x + 80 , head->pos.y - tileSize/2 - 60, msg, trueRed, 20, "data/indiana.ttf");
		} else {
			if(indy->grabbing){
				text_write_raw_ttf(app->screen, indy->door.x , indy->door.y - tileSize/2 - 20, "I", trueRed, 60, "data/arrows.ttf");
				text_write_raw_ttf(app->screen, indy->door.x + 60, indy->door.y - tileSize/2 -40, "DROP", trueRed, 20, "data/indiana.ttf");
			} else {
				text_write_raw_ttf(app->screen, allan->door.x -tileSize/2 - 60 , allan->door.y - tileSize/2 - 20, "F", trueRed, 60, "data/arrows.ttf");
				text_write_raw_ttf(app->screen, allan->door.x - tileSize * 4 - 20, allan->door.y - tileSize/2 + 40, "DROP", trueRed, 20, "data/indiana.ttf");
			}
		
		}
	}

}

