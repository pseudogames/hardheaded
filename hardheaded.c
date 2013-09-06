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
#include "sound.h"
#include "config.h"

#define FPS 30

#define MAX(a,b) ((a)>(b)?(a):(b))

#include "data/map01.h"
#include "data/map01_hit.h"
#include "data/indiana-low.h"

void loadMap(App *app) {
  char image_path[256];
  char hit_path[256];
  char wave_path[256];
  app->game.board.image = IMG_Load_RW(SDL_RWFromConstMem(map01_png,     map01_png_len),0);
  app->game.board.hit   = IMG_Load_RW(SDL_RWFromConstMem(map01_hit_png, map01_hit_png_len),0);

  moveInit(app);
}

void boardInit(App *app){
	app->game.board.spawnTime = app->game.start;
	app->game.board.kill_count= 0;
	memset(app->game.board.death1, 0, sizeof(app->game.board.death1));
	memset(app->game.board.death2, 0, sizeof(app->game.board.death2));

	app->game.board.latest_enemy_updated = 0;

	memset(&app->game.board.enemies, 0, sizeof(app->game.board.enemies));

	movePrepare(app);
}


void gameInit(App *app){
  app->game.start = SDL_GetTicks();
  app->game.next_wave = app->game.start+5000;
  app->game.wave_index = -1;
  app->game.wave_count = 4;

  app->game.wave[0].time=30000;
  app->game.wave[0].enemy_spawn_interval=3000;
  app->game.wave[0].enemy_count=100;
  app->game.wave[0].enemy_count_on_screen=50;
  app->game.wave[0].enemy_count_per_spawn=20;
  app->game.wave[0].enemy_variation=2;

  app->game.wave[1].time=60000;
  app->game.wave[1].enemy_spawn_interval=2000;
  app->game.wave[1].enemy_count=140;
  app->game.wave[1].enemy_count_on_screen=70;
  app->game.wave[1].enemy_count_per_spawn=30;
  app->game.wave[1].enemy_variation=3;

  app->game.wave[2].time=90000;
  app->game.wave[2].enemy_spawn_interval=1000;
  app->game.wave[2].enemy_count=200;
  app->game.wave[2].enemy_count_on_screen=100;
  app->game.wave[2].enemy_count_per_spawn=40;
  app->game.wave[2].enemy_variation=5;

  app->game.wave[3].time=100000;
  app->game.wave[3].enemy_spawn_interval=250;
  app->game.wave[3].enemy_count=300;
  app->game.wave[3].enemy_count_on_screen=150;
  app->game.wave[3].enemy_count_per_spawn=50;
  app->game.wave[3].enemy_variation=8;

  app->game.total_kill_count= 0;
  app->game.winner = NULL;
  app->game.board.zombie_memory1 = 0;
  app->game.board.zombie_memory2 = 0;

  app->game.indy.name = "Mr. Indy J.";
  app->game.indy.body.damage = PLAYER_DAMAGE;
  app->game.indy.body.life = PLAYER_HEALTH;
  app->game.indy.body.kills = 0;
  app->game.indy.body.score = 0;
  app->game.indy.body.action= ACTION_MOVE;
  app->game.indy.body.frame= 0;
  app->game.indy.body.ang_vel = 0.25;
  app->game.indy.body.max_vel = 4;
  app->game.indy.special_attack = 0;
  app->game.indy.grabbing = 0;

  app->game.allan.name = "Mr. Allan Q.";
  app->game.allan.body.damage = PLAYER_DAMAGE;
  app->game.allan.body.life = PLAYER_HEALTH;
  app->game.allan.body.kills = 0;
  app->game.allan.body.action= ACTION_MOVE;
  app->game.allan.body.frame= 0;
  app->game.allan.body.score = 0;
  app->game.allan.body.ang_vel = 0.25;
  app->game.allan.body.max_vel = 4;
  app->game.allan.special_attack = 0;
  app->game.allan.grabbing = 0;

  app->game.head.body.life = HEAD_HEALTH;
  app->game.head.body.score = 0;
  app->game.head.body.ang_vel = 0.25;
  app->game.head.body.max_vel= 4;

  loadMap(app);
  boardInit(app);
}

void setWave(App *app, int wave_index) {
	playNewWave();
	app->game.wave_index = wave_index;
	if(app->game.wave_index > app->game.wave_count-1)
		app->game.wave_index = app->game.wave_count-1;
	app->game.board.wave_start = SDL_GetTicks();
	app->game.board.total_enemies = 0;
	app->game.board.on_screen_enemies = 0;
	app->game.next_wave = SDL_GetTicks() +
		app->game.wave[ app->game.wave_index ].time;

}


void appInit(App *app){
  srand(time(NULL));
  memset(app, 0, sizeof(App));
  soundInit();
  app->state = STATE_MENU;
  app->menu.selected = MENU_NEW_GAME;
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
  gameInit(&app);
  conf_keys_load("keys.ini");
  loadSFX();
  init_font();

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

  while(app.state != STATE_EXIT){
	  Uint32 startTime = SDL_GetTicks();
	  Uint32 color = SDL_MapRGB(app.screen->format, 0x40, 0x40, 0x40 );
	  SDL_FillRect(app.screen, NULL , color);
	  movePrepare(&app);
	  bindKeyboard(&app);

	  if(app.credits){
		  renderCredits(&app);
	  } else {
		  switch(app.state){
			  case STATE_PLAYING:
				  if(startTime > app.game.next_wave)
					  setWave(&app, app.game.wave_index+1);    
				  spawnEnemy(&app);
				  moveEnemies(&app);
				  renderGameplay(&app);
				  break;
			  case STATE_PAUSED:
			  case STATE_MENU:
				  playMusic(indiana_low_ogg, indiana_low_ogg_len, -1);
				  renderMenu(&app);
				  break;
		  }
	  }
	  
	  SDL_Flip(app.screen);
	  handleDelay(startTime);
  }

  renderTerminate(&app);
}

