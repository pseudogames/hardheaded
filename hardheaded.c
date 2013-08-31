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

#define FPS 30


void renderInit(App *app){
  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
  app->logo = IMG_Load("data/logo.png");
  app->menu.indiana = IMG_Load("data/goldhead.jpg");
}

void appInit(App *app){
  app->state = STATE_MENU;
  app->menu.selected = MENU_NEW_GAME;
  memset(app, 0, sizeof(app));
}

int main(int argc, char* args[]) {
  App app;
  appInit(&app);
  renderInit(&app);

  init_font();

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

  while(app.state != STATE_EXIT){
	  bindKeyboard(&app);
	  Uint32 color = SDL_MapRGB(app.screen->format, 44, 0,0 );
	  SDL_FillRect(app.screen, NULL , color);

	  switch(app.state){
		  case STATE_PLAYING:
			  renderGameplay(&app);
			  break;
		  case STATE_PAUSED:
		  case STATE_MENU:
			  renderMenu(&app);
			  break;
	  }
  }
}

