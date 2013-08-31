#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#include "hardheaded.h"
#include "keyboard.h"
#include "font.h"
#include "gameplay.h"

#include "sprite.h"

#define FPS 30

#define MAX(a,b) ((a)>(b)?(a):(b))

void renderInit(App *app){
  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
  app->logo = IMG_Load("data/logo.png");
  app->menu.indiana = IMG_Load("data/goldhead.jpg");
  app->game.board.hearts.full = IMG_Load("data/fullheart_small.png");
  app->game.board.hearts.onequarter= IMG_Load("data/14heart_small.png");
  app->game.board.hearts.twoquarter= IMG_Load("data/24heart_small.png");
  app->game.board.hearts.threequarter= IMG_Load("data/34heart_small.png");
  app->game.board.hearts.empty = IMG_Load("data/emptyheart_small.png");

  sprite_init(&app->game.board.indy.sprite, 
	  0, 0, // origin
	  32, 64, 4, // frame size and count
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

void appInit(App *app){
  memset(app, 0, sizeof(App));
  app->state = STATE_MENU;
  app->menu.selected = MENU_NEW_GAME;
  app->game.board.indy.name = "Mr. Indy J.";
  app->game.board.indy.life= 10.0;
  app->game.board.allan.name = "Mr. Allan Q.";
  app->game.board.allan.life= 10.0;
}

void sprite_demo(App *app, Body *body) {
	  body->frame += 0.2;
	  if((int)body->frame > body->sprite->frame_count) {
		  body->frame = 0;

		  if((rand() % 10) == 0) {
			  body->action ++;
			  if((int)body->action >= ACTION_COUNT)
				  body->action = ACTION_MOVE;
		  }
	  }

	  body->angle += 1;
	  if((int)body->angle >= 360)
		  body->angle = 0;

      SDL_Rect src;
	  sprite_rotated_rect(
			  body->sprite, 
			  body->action, 
			  (int)body->frame,
			  (int)body->angle,
			  &src);

	  SDL_BlitSurface(body->sprite->rotated, &src, app->screen, &body->pos);
}

void handleDelay(Uint32 start) {
  Uint32 end = SDL_GetTicks();
  int actual_delta = end - start;
  int expected_delta = 1000/FPS;
  int delay = MAX(0, expected_delta - actual_delta);
  //printf("delay %d %d %d\n", actual_delta, expected_delta, delay);
  SDL_Delay(delay);
}


int main(int argc, char* args[]) {
  App app;
  appInit(&app);
  renderInit(&app);

  init_font();

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

  while(app.state != STATE_EXIT){
	  Uint32 startTime = SDL_GetTicks();
	  Uint32 color = SDL_MapRGB(app.screen->format, 0x40, 0x40, 0x40 );
	  SDL_FillRect(app.screen, NULL , color);
	  bindKeyboard(&app);

	  switch(app.state){
		  case STATE_PLAYING:
			  renderGameplay(&app);
			  break;
		  case STATE_PAUSED:
		  case STATE_MENU:
			  renderMenu(&app);
			  break;
	  }

	  sprite_demo(&app, &app.game.board.indy.body);
	  sprite_demo(&app, &app.game.board.zombie.body);
	  sprite_demo(&app, &app.game.board.head.body);

	  SDL_Flip(app.screen);
	  handleDelay(startTime);
  }
}

