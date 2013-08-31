#include <SDL_image.h>
#include <math.h>
#include "render.h"
#include "font.h"
#include "aStarLibrary.h"

SDL_Color red = {0xAA, 0X55, 0x00};
SDL_Color trueRed = {0XFF, 0x00, 0x00};
SDL_Color white = {0xFF, 0XFF, 0xFF};
SDL_Color green = {0x00, 0XFF, 0x00};
SDL_Color yellow = {0xFF, 0XFF, 0x00};

//// sprite

void sprite_origin_rect(Sprite *sprite, Action action, int frame, SDL_Rect *rect)
{
    frame = frame % sprite->frame_count;
    rect->x = sprite->origin.x + frame *sprite->frame_size.x;
    rect->y = sprite->origin.y + action*sprite->frame_size.y;
    rect->w = sprite->frame_size.x;
    rect->h = sprite->frame_size.y;
}

#define ANGLE_STEP 30
#define ZOOM 1

void sprite_rotated_rect(Sprite *sprite, Action action, int frame, int angle, SDL_Rect *rect)
{
    frame = frame % sprite->frame_count;
    int angle_index = ((int)(360+angle+ANGLE_STEP/2) % 360) / ANGLE_STEP;
    rect->x = frame *sprite->rotated_frame_size.x;
    rect->y = action*sprite->rotated_frame_size.y+
        + sprite->rotated_frame_size.y*ACTION_COUNT*angle_index;
    rect->w = sprite->rotated_frame_size.x;
    rect->h = sprite->rotated_frame_size.y;
}

void sprite_gen_rotation(Sprite *sprite)
{
    rotozoomSurfaceSize(
            sprite->frame_size.x,
            sprite->frame_size.y,
            45, // to maximize size
            ZOOM,  // no zoom
            &sprite->rotated_frame_size.x,
            &sprite->rotated_frame_size.y
            );

    if(sprite->rotated)
        SDL_FreeSurface(sprite->rotated);

    sprite->rotated = SDL_CreateRGBSurface(SDL_HWSURFACE, 
            sprite->rotated_frame_size.x * sprite->frame_count,
            sprite->rotated_frame_size.y * ACTION_COUNT * 360/ANGLE_STEP,
            RGBA_FORMAT);
    if(sprite->rotated == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    //printf("cache size %dx%d for %d angles\n", sprite->rotated->w, sprite->rotated->h, 360/ANGLE_STEP);

    SDL_Surface *element = SDL_CreateRGBSurface(SDL_HWSURFACE, 
            sprite->frame_size.x, 
            sprite->frame_size.y,
            RGBA_FORMAT);

    SDL_SetAlpha(sprite->source,0,0xff);
    SDL_SetAlpha(element,0,0xff);
    SDL_SetAlpha(sprite->rotated,SDL_SRCALPHA,0xff);

    int frame, action, angle;
    for(action=0; action<ACTION_COUNT; action++) {
        for(frame=0; frame<sprite->frame_count; frame++) {
            SDL_Rect src;
            sprite_origin_rect(sprite, action, frame, &src);
            for(angle=0; angle<360; angle+=ANGLE_STEP) {
                SDL_Rect dst;
                sprite_rotated_rect(sprite, action, frame, angle, &dst);
                SDL_FillRect(element, NULL, 0x00000000);
                SDL_BlitSurface( sprite->source, &src, element, NULL );
                SDL_Surface *rotozoom = rotozoomSurface(element, angle, ZOOM, SMOOTHING_ON);
                SDL_SetAlpha(rotozoom,0,0);
                SDL_SetColorKey(rotozoom,0,0);
                dst.x += dst.w/2 - rotozoom->w/2;
                dst.y += dst.h/2 - rotozoom->h/2; // center
                SDL_BlitSurface(rotozoom, NULL, sprite->rotated, &dst );
                SDL_FreeSurface(rotozoom);
            }
        }
    }

    SDL_FreeSurface(element);
}

void sprite_init(Sprite *sprite, int ox, int oy, int fx, int fy, int c, char *filename)
{
    memset(sprite,0,sizeof(Sprite));
    sprite->origin.x = ox;
    sprite->origin.y = oy;
    sprite->frame_size.x = fx;
    sprite->frame_size.y = fy;
    sprite->frame_count = c;
    sprite->source = IMG_Load( filename );
    sprite->rotated = NULL;
    sprite_gen_rotation(sprite);
}


//// end sprite


void renderStats(App *app, SDL_Surface *screen, Player *player1, Player *player2){
 int t = SDL_GetTicks();
  if(player1->body.status != BODY_DEAD){
    text_write_raw(screen, 5, 60, "Player 1", green, 30);

      char ammo_label[256];
	  int ammo = player1->body.item.type->ammo_total - player1->body.item.ammo_used;
      sprintf(ammo_label, "ammo: %i", ammo) ;
      text_write_raw(screen, 5, 30, ammo_label, ammo < 200 ? ( (t/200) % 2 ? trueRed : white) : yellow, 20);

      Uint32 color = SDL_MapRGB(screen->format, 99, 0,0 );
      SDL_Rect rect = { 5, 10, player1->body.life*2, 20};
      SDL_FillRect(screen, &rect, color);
  } else {
	  text_write_raw(screen, 5, 10, "Soldier Press Start", (t/500) % 2 ? yellow : red, 20);
  }

  char kills[256];
  sprintf(kills, "%i Total kill(s)", app->game.total_kill_count);
  text_write_raw(screen, 400, 30, kills, white, 40);

  char enemies_wave[256];
  int expected_kills = app->game.board.wave[app->game.board.wave_index].enemy_count;
  sprintf(enemies_wave, "Wave %d: %d/%d Enemies", 
		  app->game.board.wave_index + 1,
		  expected_kills - app->game.kill_count,
		  expected_kills
  );
  text_write_raw(screen, 360, 90, enemies_wave, green, 30);


  if(t < app->game.board.wave_start + 5000) {
	  char new_wave[256];
	  sprintf(new_wave, "NEW WAVE IN %d", 5-(t-app->game.board.wave_start) / 1000);
	  text_write_raw(screen, 360, 420, new_wave, (t/500) % 2 ? trueRed : white, 60);
  }

  if(player2->body.status != BODY_DEAD){
    text_write_raw(screen, 900, 60, "Player 2", green, 30);

      char ammo_label[256];
	  int ammo = player2->body.item.type->ammo_total - player2->body.item.ammo_used;
      sprintf(ammo_label, "ammo: %i", ammo) ;
      text_write_raw(screen, 800, 30, ammo_label, ammo < 200 ? ( (t/200) % 2 ? trueRed : white) : yellow, 20);

	  Uint32 color = SDL_MapRGB(screen->format, 0xff, 0,0 );
	  SDL_Rect rect = { 800, 10, player2->body.life*2, 20};
	  SDL_FillRect(screen, &rect, color);
	}else {
	  text_write_raw(screen, 780, 10, "Engineer Press Start", (t/500) % 2 ? yellow : red, 20);
	}
}

void renderBody(Game *game, Body *body){
	if(body->status != BODY_ALIVE) return;

	int j = game->board.blit_count++;
	game->board.blit[j].image = body->sprite->image;
	
    sprite_rotated_rect(
		body->sprite, 
		body->action, 
		body->frame,
		body->angle,
		&game->board.blit[j].src);

	int w = game->board.blit[j].src.w;
	int h = game->board.blit[j].src.h;

	SDL_Rect dst = {
		body->pos.x - w/2,
		body->pos.y - h*3/4,
		w, h
	};

	game->board.blit[j].dst = dst;
}

void renderEnemies(App *app)
{
  int i;
  for(i = 0; i < ENEMY_COUNT; i++) {
    renderBody(game, &app->game.enemies[i].body);
  }
}

int blit_zsort(const void *a, const void *b)
{
	Blit *aa = (Blit *)a;
	Blit *bb = (Blit *)b;
	return aa->rect.y - bb->rect.y;
}

void flushRender(App *app)
{
	int i;
	qsort(app->game.board.blit, app->game.board.blit_count, sizeof(Blit), blit_zsort);

	for(i=0; i< app->game.board.blit_count; i++) {
		SDL_BlitSurface(app->game.board.blit[i].image, &app->game.board.blit[i].src, app->screen, &app->game.board.blit[i].dst);
	}
}

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
		case DEBUG_ITEM: // magenta
			color = SDL_MapRGBA(app->screen->format, 0xff,0x00,0xff,0xff );
			map = (int *)app->game.board.powerup;
			break;
		case DEBUG_SAFE: // cyan
			color = SDL_MapRGBA(app->screen->format, 0x00,0xff,0xff,0xff );
			map = (int *)app->game.board.safearea;
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
			if(app->game.enemies[i].body.status == BODY_ALIVE)
			{
				extern int pathBank [numberPeople+1][maxPathLength*2];
				int p = app->game.enemies[i].pathfinder;
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


void renderStart(App *app){
  app->game.board.blit_count = 0;
  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );

  Game game = app->game;

  int t = SDL_GetTicks();
  if(t < app->game.board.wave_start + 2000) {
	  int alpha = 0xff * (t-app->game.board.wave_start)/2000;
	  SDL_SetAlpha(game.board.image, SDL_SRCALPHA, alpha);
	  // printf("wave start %d\n", alpha);
  } else {
	  SDL_FillRect(app->screen, NULL , color);
	  SDL_SetAlpha(game.board.image, 0, 0xff);
  }
  SDL_BlitSurface(app->game.board.image, NULL, app->screen, NULL);

  renderDebug(app);

}

void renderFinish(App *app){
  Game game = app->game;
  int t = SDL_GetTicks();

  renderBody(&app->game, &game.player1.body);

  if(t < app->game.hint_pivot) {
		Player *player = app->game.player1.body.status==BODY_ALIVE ? &app->game.player1 : &app->game.player2;
	  text_write_raw(app->screen, player->body.pos.x-60, player->body.pos.y+20, "button2 to aim!!", (t/300) % 2 ? yellow : red, 20);
  }

  renderBody(&app->game, &game.player2.body);
  renderEnemies(app);

  flushRender(app);
  renderStats(app, app->screen, &game.player1, &game.player2);

  SDL_Flip(app->screen);
}

void renderInit(App *app){

    sprite_init(&app.game.board.indy, 
            0, 0, // origin
            64, 64, 8, // frame size and count
            "indy.png" // source
		);

    sprite_init(&app.game.board.alan, 
            0, 0, // origin
            64, 64, 8, // frame size and count
            "alan.png" // source
		);

    sprite_init(&app.game.board.zombie, 
            0, 0, // origin
            64, 64, 8, // frame size and count
            "zombie.png" // source
		);

    sprite_init(&app.game.board.head, 
            0, 0, // origin
            64, 64, 8, // frame size and count
            "head.png" // source
		);



  app->logo = IMG_Load("data/logo.png");
  app->menu.soldier = IMG_Load("data/soldado1_grande.png");
  app->menu.bigZombie = IMG_Load("data/zombie2_grande.png");
  app->menu.keyBinds = IMG_Load("data/keyboard-layout.png");
  app->menu.zombie = IMG_Load("data/zombie1.png");
  app->menu.engineer = IMG_Load("data/engenheiro1.png");

  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE
#if RELEASE
	|SDL_FULLSCREEN
#endif
  );
}

void renderTerminate(App *app){
	SDL_FreeSurface(app->logo);
	SDL_FreeSurface(app->menu.soldier);
	SDL_FreeSurface(app->menu.bigZombie);
	SDL_FreeSurface(app->menu.keyBinds);
	SDL_FreeSurface(app->menu.zombie);
	SDL_FreeSurface(app->menu.engineer);

}

/**
 *
 * TODO: refactor this to be part of render func
 */
void renderMenu(App *app){
	Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
	SDL_FillRect(app->screen, NULL , color);
	Menu *menu = &app->menu;
	SDL_Surface *screen = app->screen;

	SDL_Rect logoPos = {-450, -650, screen->w, screen->h};
	SDL_BlitSurface(app->logo, &logoPos, screen, NULL);

    text_write_raw(screen, -450, -700, "Ola mundo", green, 30);

	SDL_Rect titleCharPos = {-700, -50, screen->w, screen->h};
	SDL_BlitSurface(menu->zombie, &titleCharPos, screen, NULL);

	if(app->state == STATE_GAMEOVER) {
	  SDL_Rect highlightChar = {-750, -450, screen->w, screen->h};
	  SDL_BlitSurface(menu->bigZombie, &highlightChar, screen, NULL);
	} else {
	  SDL_Rect highlightChar = {-700, -450, screen->w, screen->h};
	  SDL_BlitSurface(menu->soldier, &highlightChar, screen, NULL);
	}

	int resumePadding = 0;

	if(app->state == STATE_GAMEOVER){
	  resumePadding = 100;

	  char kills[256];
	  SDL_Color color;
	  if(app->game.won) {
		  color = green;
		  sprintf(kills, "You won!!! Zombies killed: %i", app->game.total_kill_count);
	  } else {
		  color = trueRed;
		  sprintf(kills, "You died. Zombies killed: %i", app->game.total_kill_count);
	  }
	  text_write_raw(screen, 100, 250, kills, color, 50);
	} else if(app->state == STATE_PAUSED){
	  resumePadding = 100;
	  text_write(screen, 100, 250, "resume game", menu->selected == MENU_RESUME);
	}

    text_write_raw(screen, 300, 50, "Survivor", red, 96);

	text_write(screen, 100, 250 + resumePadding, "new game", menu->selected == MENU_NEW_GAME);
	text_write(screen, 100, 350 + resumePadding, "help", menu->selected == MENU_HELP);
	text_write(screen, 100, 450 + resumePadding, "credits", menu->selected == MENU_CREDITS);
	text_write(screen, 100, 550 + resumePadding, "exit", menu->selected == MENU_QUIT);

  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderHelp(App *app)
{
	Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
	SDL_Surface *screen = app->screen;
	SDL_FillRect(screen, NULL , color);

	SDL_Rect keyBindRect = {150, 100, screen->w, screen->h};
	SDL_BlitSurface(app->menu.keyBinds, NULL, screen, &keyBindRect);

	text_write_raw(screen, 300, 50, "Help", red, 96);

  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderCredits(App *app)
{
  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_Surface *screen = app->screen;
  SDL_FillRect(screen, NULL , color);

  SDL_Rect charPos = {-670, -50, screen->w, screen->h};
  SDL_BlitSurface(app->menu.engineer, &charPos, screen, NULL );

  text_write_raw(screen, 300, 50, "Credits", red, 96);

  if(app->credits != CREDITS_SOUND){
	text_write_raw(screen, 100, 150, "team", green, 36);
	text_write_raw(screen, 100, 200, "Carlo \"zED\" Caputo", white, 26);
	text_write_raw(screen, 100, 250, "Pedro Mariano", white, 26);
	text_write_raw(screen, 100, 300, "Caires Vinicius", white, 26);

	text_write_raw(screen, 100, 400, "chars", green, 36);
	text_write_raw(screen, 100, 450, "based on http://pixelblock.tumblr.com", white, 26);
	text_write_raw(screen, 100, 500, "modified by Pedro Jatoba", white, 26);

	text_write_raw(screen, 100, 600, "tileset", green, 36);
	text_write_raw(screen, 100, 650, "lost garden", white, 26);
	text_write_raw(screen, 400, 600, "font", green, 36);
	text_write_raw(screen, 400, 650, "Pixelsix, 2005 by Cal Henderson", white, 26);
  } else {
	text_write_raw(screen, 100, 150, "Musics", green, 36);
	text_write_raw(screen, 100, 200, "cluck, Computer Savvy - The J Arthut Keenes band", white, 26);
	text_write_raw(screen, 100, 250, "Come and Find Us Remix, Resistor Anthems - Eric Skiff", white, 26);
	text_write_raw(screen, 100, 300, "Arpanauts, Resistor Anthems - Eric Skiff", white, 26);
	text_write_raw(screen, 100, 350, "Underclocked (mix), Resistor Anthems - Eric Skiff", white, 26);

	text_write_raw(screen, 100, 450, "Sounds", green, 36);

	text_write_raw(screen, 100, 500, "http://www.freesound.org/people/LAGtheNoggin/sounds/15545/", white, 26);
	text_write_raw(screen, 100, 550, "http://www.freesound.org/people/Sparrer/sounds/50506/", white, 26);
	text_write_raw(screen, 100, 600, "http://www.freesound.org/people/DJ20Chronos/sounds/33380/", white, 26);
	text_write_raw(screen, 100, 650, "http://www.freesound.org/people/WIM/sounds/17918/", white, 26);
  }

  SDL_UpdateRect(screen, 0, 0, 0, 0);
}



