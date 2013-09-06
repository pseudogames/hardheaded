#include <SDL_image.h>
#include <math.h>
#include "render.h"

#include "data/blood.h"
#include "data/logo.h"
#include "data/indy-idol.h"
#include "data/fullheart_small.h"
#include "data/14heart_small.h"
#include "data/24heart_small.h"
#include "data/34heart_small.h"
#include "data/emptyheart_small.h"
#include "data/chargebarr.h"
#include "data/patrocinador.h"

#include "data/indy.h"
#include "data/allan.h"
#include "data/zombie.h"
#include "data/head.h"
#include "data/power.h"

#include "data/indiana.h"
#include "data/arrows.h"



SDL_Color red = {0xAA, 0X55, 0x00};
SDL_Color trueRed = {0XFF, 0x00, 0x00};
SDL_Color white = {0xFF, 0XFF, 0xFF};
SDL_Color green = {0x00, 0XFF, 0x00};
SDL_Color yellow = {0xFF, 0XFF, 0x00};
SDL_Color black = {0x00, 0X00, 0x00};

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
		player->power_body.vel = tileSize*1.5;


		if(player->power_body.action == ACTION_ATTACK && player->special_attack < 98){
			float a = player->power_body.angle * M_PI / 180;
			float dx = cos(a) * player->power_body.vel;
			float dy = sin(a) * player->power_body.vel;
			float tx = player->power_body.pos.x + dx;
			float ty = player->power_body.pos.y - dy;

			player->power_body.frame ++;

			shoot(app, &player->power_body, 100, 0, player->power_body.vel*2);
			shoot(app, &player->power_body, 100, 0, player->power_body.vel*2);
			shoot(app, &player->power_body, 100, 0, player->power_body.vel*2);

			player->body.life += player->power_body.life;
			player->body.kills += player->power_body.kills;
			player->power_body.life = 0;
			player->power_body.kills = 0;

			if(!is_air(&app->game, &player->power_body, (int)tx, (int)ty)){
				player->power_body.action = ACTION_DEATH;
				player->power_body.frame = 0;
			}

			player->power_body.pos.x = tx;
			player->power_body.pos.y = ty;
		} else {
			float a = player->body.angle * M_PI / 180;
			float dx = cos(a) * player->power_body.vel;
			float dy = sin(a) * player->power_body.vel;
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
  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE |SDL_DOUBLEBUF
#if RELEASE
		  |SDL_FULLSCREEN
#endif
  );
  app->blood =              IMG_Load_RW(SDL_RWFromConstMem(blood_png,            blood_png_len),           0);
  app->logo =               IMG_Load_RW(SDL_RWFromConstMem(logo_png,             logo_png_len),            0);
  app->menu.indiana =       IMG_Load_RW(SDL_RWFromConstMem(indy_idol_png,        indy_idol_png_len),       0);
  app->hearts.full =        IMG_Load_RW(SDL_RWFromConstMem(fullheart_small_png,  fullheart_small_png_len), 0);
  app->hearts.onequarter=   IMG_Load_RW(SDL_RWFromConstMem(__14heart_small_png,  __14heart_small_png_len), 0);
  app->hearts.twoquarter=   IMG_Load_RW(SDL_RWFromConstMem(__24heart_small_png,  __24heart_small_png_len), 0);
  app->hearts.threequarter= IMG_Load_RW(SDL_RWFromConstMem(__34heart_small_png,  __34heart_small_png_len), 0);
  app->hearts.empty =       IMG_Load_RW(SDL_RWFromConstMem(emptyheart_small_png, emptyheart_small_png_len),0);
  app->special_bar =        IMG_Load_RW(SDL_RWFromConstMem(chargebarr_png,       chargebarr_png_len),      0);
  app->partnership =        IMG_Load_RW(SDL_RWFromConstMem(patrocinador_jpg,     patrocinador_jpg_len),    0);

  sprite_init(&app->game.indy.sprite, 
	  0, 0, // origin
	  32, 96, 4, // frame size and count
	  indy_png, indy_png_len // source
  );
  app->game.indy.body.sprite = &app->game.indy.sprite;

  sprite_init(&app->game.allan.sprite, 
	  0, 0, // origin
	  32, 96, 4, // frame size and count
	  allan_png, allan_png_len // source
  );
  app->game.allan.body.sprite = &app->game.allan.sprite;

  app->enemy_frame_count = 3;
  app->enemy_variation_count = 8;
  sprite_init(&app->zombie, 
	  0, 0, // origin
	  32, 32, app->enemy_frame_count * app->enemy_variation_count, // frame size and count
	  zombie_png, zombie_png_len // source
  );

  sprite_init(&app->game.head.sprite, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  head_png, head_png_len // source
  );

  app->game.head.body.sprite = &app->game.head.sprite;

  sprite_init(&app->game.allan.power, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  power_png, power_png_len // source
  );

  app->game.allan.power_body.sprite = &app->game.allan.power;


  sprite_init(&app->game.indy.power, 
	  0, 0, // origin
	  32, 32, 3, // frame size and count
	  power_png, power_png_len // source
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
	if(player->body.life <= PLAYER_HEALTH * 0.43 && (SDL_GetTicks()/500) % 2 == 0){
		return;
	}

	text_write_raw(screen, 35 + playerOffset, 10, player->name, yellow, 20);
	{
		char buf[256];
		sprintf(buf,"kills %d", player->body.kills);
		text_write_raw(screen, 220 + playerOffset, 10, buf, white, 12);
	}
	for(i = 0; i < PLAYER_HEALTH; i++){
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
	text_write_raw(app->screen, 20 , 330, msg, white, 61);
}

void renderGameOver(App *app){
	text_write_raw(app->screen, 150 , 330, "Draw. Game Over.", white, 61);
}
void renderHints(App *app){
	if(app->game.head.body.life <= 0 || app->game.winner) return;
	Body *head = &app->game.head.body;
	Player *indy = &app->game.indy;
	Player *allan = &app->game.allan;

	if((SDL_GetTicks()/500) % 2 == 0){

		if(indy->grabbing == 0 && allan->grabbing == 0){
			char *msg = "GRAB";
			if(indy->body.action == ACTION_DEATH || indy->body.action == ACTION_DEATH) msg = "GUARD";
			text_write_raw_ttf(app->screen, head->pos.x , head->pos.y - tileSize/2 - 60, "I", trueRed, 60, arrows_ttf, arrows_ttf_len);
			text_write_raw_ttf(app->screen, head->pos.x + 80 , head->pos.y - tileSize/2 - 60, msg, trueRed, 20, indiana_ttf, indiana_ttf_len);
		} else {
			if(indy->grabbing){
				text_write_raw_ttf(app->screen, indy->door.x , indy->door.y - tileSize/2 - 20, "I", trueRed, 60, arrows_ttf, arrows_ttf_len);
				text_write_raw_ttf(app->screen, indy->door.x + 60, indy->door.y - tileSize/2 -40, "DROP", trueRed, 20, indiana_ttf, indiana_ttf_len);
			} else {
				text_write_raw_ttf(app->screen, allan->door.x -tileSize/2 - 60 , allan->door.y - tileSize/2 - 20, "F", trueRed, 60, arrows_ttf, arrows_ttf_len);
				text_write_raw_ttf(app->screen, allan->door.x - tileSize * 4 - 20, allan->door.y - tileSize/2 + 40, "DROP", trueRed, 20, indiana_ttf, indiana_ttf_len);
			}
		
		}
	}

}

void renderCredits(App *app)
{
	Uint32 color = SDL_MapRGB(app->screen->format, 0xc1, 0xcd,0xc1);
	SDL_Surface *screen = app->screen;
	SDL_FillRect(screen, NULL , color);

	SDL_Rect charPos = {-670, -50, screen->w, screen->h};
//	SDL_BlitSurface(app->menu.engineer, &charPos, screen, NULL );

	text_write_raw(screen, 300, 25, "Credits", black, 96);

	if(app->credits ==1){
		text_write_raw(screen, 100, 160, "team", red, 36);
		text_write_raw(screen, 100, 200, "Programmer: Carlo \"zED\" Caputo", black, 20);
		text_write_raw(screen, 100, 240, "Programmer: Pedro Mariano", black, 20);
		text_write_raw(screen, 100, 280, "SFX: Maxwell Dayvson", black, 20);

		text_write_raw(screen, 100, 320, "chars based on:", red, 36);
		text_write_raw(screen, 100, 360, "sprite database (players)", red, 20);
		text_write_raw(screen, 100, 400, "http://spritedatabase.net/files/pc/771/Sprite/IndianaJones.png", black, 20);

		text_write_raw(screen, 100, 440, "spelunky (head)", red, 20);
		text_write_raw(screen, 100, 470, "http://spelunky.wikia.com/wiki/Golden_Idol/Classic", black, 20);

		text_write_raw(screen, 100, 500, "Kazzador/Enterbrail (enemies)", red, 20);
		text_write_raw(screen, 100, 530, "http://forums.rpgmakerweb.com/index.php?/topic/", black, 20);
		text_write_raw(screen, 100, 575, "683-kazzadors-additional-generator-parts-sprites/", black, 20);

		text_write_raw(screen, 100, 600, "tileset", red, 36);
		text_write_raw(screen, 100, 640, "The Legend of Zelda: The Minish Cap", red, 20);
		text_write_raw(screen, 100, 670, "http://www.pyxosoft.com/media/resources/loz_minish_cap/", black, 20);
	} else if(app->credits == 2) {
		text_write_raw(screen, 100, 180, "music", red, 36);
		text_write_raw(screen, 100, 230, "Indiana Jones theme 8bits", red, 20);
		text_write_raw(screen, 100, 260, "http://www.newgrounds.com/audio/listen/148265", black, 20);
		text_write_raw(screen, 100, 290, "sfx", red, 36);
		text_write_raw(screen, 100, 340, "Indy death (wilhem scream)", red, 20);
		text_write_raw(screen, 100, 370, "http://www.freesound.org/people/Syna-Max/sounds/64940/", black, 20);

		text_write_raw(screen, 100, 400, "Allan death(howie scream)", red, 20);
		text_write_raw(screen, 100, 430, "http://www.freesound.org/people/M3GAMAN/sounds/197914/", black, 20);

		text_write_raw(screen, 100, 460, "Special attack", red, 20);
		text_write_raw(screen, 100, 490, "http://www.freesound.org/people/ejfortin/sounds/49694/", black, 20);

		text_write_raw(screen, 100, 520, "whip miss", red, 20);
		text_write_raw(screen, 100, 550, "http://www.freesound.org/people/goldendiaphragm/sounds/119117/", black, 20);

		text_write_raw(screen, 100, 580, "whip hit", red, 20);
		text_write_raw(screen, 100, 610, "http://www.freesound.org/people/Syna-Max/sounds/43586/", black, 20);

		text_write_raw(screen, 100, 630, "wake up", red, 20);
		text_write_raw(screen, 100, 650, "http://www.freesound.org/people/thecluegeek/sounds/140585/", black, 20);
	} else if(app->credits == 3){
		text_write_raw(screen, 100, 180, "fonts", red, 36);
		text_write_raw(screen, 100, 230, "http://www.dafont.com/indiana.font", black, 20);
		text_write_raw(screen, 100, 260, "http://www.dafont.com/drawing-blood.font", black, 20);
		text_write_raw(screen, 100, 290, "http://www.dafont.com/pwnewarrows.font", black, 20);
	}else if(app->credits == 4){
		text_write_raw(screen, 130, 170, "Este jogo AWESOME foi desenvolvido durante a SPJam 2013", black, 20);
		text_write_raw(screen, 220, 210, "realizada pela Vortex Game Studios na PUC-SP", black, 20);
		SDL_Rect logo = {-220, -250, screen->w, screen->h};
		SDL_BlitSurface(app->partnership, &logo, screen, NULL);
	}
}


