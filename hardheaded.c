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
#include "render.h"

#define FPS 30

#define MAX(a,b) ((a)>(b)?(a):(b))

void appInit(App *app){
  memset(app, 0, sizeof(App));
  app->state = STATE_MENU;
  app->menu.selected = MENU_NEW_GAME;
  app->game.board.indy.name = "Mr. Indy J.";
  app->game.board.indy.life= 10;
  app->game.board.allan.name = "Mr. Allan Q.";
  app->game.board.allan.life= 10;

  app->game.board.indy.body.ang_vel = 0.25;
  app->game.board.indy.body.max_vel= 4;

  app->game.board.allan.body.ang_vel = 0.25;
  app->game.board.allan.body.max_vel= 4;
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


	  SDL_Flip(app.screen);
	  handleDelay(startTime);
  }
}

