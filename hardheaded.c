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

void loadMap(App *app) {
  char image_path[256];
  char hit_path[256];
  char wave_path[256];
  char *map_name = "map01";
  sprintf(image_path, "data/%s.png", map_name);
  sprintf(hit_path, "data/%s_hit.png", map_name);
  app->game.board.image = IMG_Load(image_path);
  app->game.board.hit = IMG_Load(hit_path);

  app->game.board.wave_count=4;

  app->game.wave[0].enemy_spawn_interval=1000;
  app->game.wave[0].enemy_count=20;
  app->game.wave[0].enemy_count_on_screen=20;
  app->game.wave[0].enemy_count_per_spawn=20;

  app->game.wave[1].enemy_spawn_interval=5000;
  app->game.wave[1].enemy_count=40;
  app->game.wave[1].enemy_count_on_screen=20;
  app->game.wave[1].enemy_count_per_spawn=10;

  app->game.wave[2].enemy_spawn_interval=2000;
  app->game.wave[2].enemy_count=120;
  app->game.wave[2].enemy_count_on_screen=80;
  app->game.wave[2].enemy_count_per_spawn=10;

  app->game.wave[3].enemy_spawn_interval=250;
  app->game.wave[3].enemy_count=120;
  app->game.wave[3].enemy_count_on_screen=30;
  app->game.wave[3].enemy_count_per_spawn=2;

}

void gameInit(App *app){
	app->game.start = SDL_GetTicks();
	app->game.spawnTime = app->game.start+5000;
	app->game.kill_count= 0;
	memset(app->game.board.death1, 0, sizeof(app->game.board.death1));
	memset(app->game.board.death2, 0, sizeof(app->game.board.death2));

	app->game.board.latest_enemy_updated = 0;

	movePrepare(app);
}

void setWave(App *app, int wave_index) {
	app->game.board.wave_index = wave_index;
	app->game.board.wave_start = SDL_GetTicks();
	app->game.total_enemies = 0;
	app->game.on_screen_enemies = 0;

	moveInit(app);
	gameInit(app);
}


void appInit(App *app){
  srand(time(NULL));
  memset(app, 0, sizeof(App));
  app->state = STATE_MENU;
  app->menu.selected = MENU_NEW_GAME;
  app->game.indy.name = "Mr. Indy J.";
  app->game.indy.body.life= 10;
  app->game.allan.name = "Mr. Allan Q.";
  app->game.allan.body.life= 10;

  app->game.indy.body.ang_vel = 0.25;
  app->game.indy.body.max_vel= 4;

  app->game.allan.body.ang_vel = 0.25;
  app->game.allan.body.max_vel= 4;

  app->game.head.body.ang_vel = 0.25;
  app->game.head.body.max_vel= 4;

  loadMap(app);
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
  setWave(&app, 0); // calls moveInit / gameInit

  init_font();

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

  while(app.state != STATE_EXIT){
	  Uint32 startTime = SDL_GetTicks();
	  Uint32 color = SDL_MapRGB(app.screen->format, 0x40, 0x40, 0x40 );
	  SDL_FillRect(app.screen, NULL , color);
	  movePrepare(&app);
	  bindKeyboard(&app);

	  switch(app.state){
		  case STATE_PLAYING:
			  spawnEnemy(&app);
			  moveEnemies(&app);
			  renderGameplay(&app);
			  break;
		  case STATE_PAUSED:
		  case STATE_MENU:
			  renderMenu(&app);
			  break;
	  }

	  renderTerminate(&app);

	  SDL_Flip(app.screen);
	  handleDelay(startTime);
  }
}

