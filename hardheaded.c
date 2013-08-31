#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#include "hardheaded.h"
#include "keyboard.h"
#include "font.h"

#define FPS 30

SDL_Color red = {0xAA, 0X55, 0x00};

void renderInit(App *app){
  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
}

int main(int argc, char* args[]) {
  App app;
  app.state = STATE_MENU;
  memset(&app, 0, sizeof(app));

  renderInit(&app);
  appInit(&app);
  init_font();

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

  while(app.state != STATE_EXIT){
	  bindKeyboard(&app);
	  Uint32 color = SDL_MapRGB(app.screen->format, 44, 0,0 );
	  SDL_FillRect(app.screen, NULL , color);
	  text_write(app.screen, 200, 200, "teste", 0);
	  SDL_UpdateRect(app.screen, 0, 0, 0, 0);
  }
}

