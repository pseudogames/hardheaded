#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

#include "font.h"
#include "app.h"
#include "sound.h"
#include "render.h"
#include "movement.h"
#include "aStarLibrary.h"

#define FPS 30
#define MAX(a,b) ((a)>(b)?(a):(b))

void finishHim(App *app){
  app->state = STATE_EXIT;
}

void checkGameover(App *app){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;

  int numCurrentPlayers = (player1->body.status != BODY_DEAD) + (player2->body.status != BODY_DEAD);
  if(!numCurrentPlayers){
	app->state = STATE_GAMEOVER;
	app->menu.selected = MENU_NEW_GAME;
  }
}

void gameInit(App *app){
	app->game.start = SDL_GetTicks();
	app->game.spawnTime = app->game.start+5000;
	app->game.spawnPowerupTime = app->game.start+15000;
	app->game.kill_count= 0;
	memset(app->game.board.powerup, 0, sizeof(app->game.board.powerup));
	memset(app->game.board.built, 0, sizeof(app->game.board.built));
	memset(app->game.board.death1, 0, sizeof(app->game.board.death1));
	memset(app->game.board.death2, 0, sizeof(app->game.board.death2));

	app->game.latest_enemy_updated = 0;

	app->credits = 0;
  int i;
  for(i=0;i < ENEMY_COUNT; i++)
  {
    app->game.enemies[i].body.status = BODY_DEAD;
  }

  for(i=0;i<POWERUP_COUNT; i++)
  {
    app->game.board.powerups[i].should_show = 0;
  }

  movePrepare(app);

	Body *p1body = &app->game.player1.body;
	player_spawn_pos(&app->game, &p1body->pos.x, &p1body->pos.y);

	Body *p2body = &app->game.player2.body;
	player_spawn_pos(&app->game, &p2body->pos.x, &p2body->pos.y);

}

void setWave(App *app, int wave_index);
void resetApp(App *app)
{
	app->game.total_kill_count= 0;
	app->game.won = 0;
	app->game.hint_pivot = SDL_GetTicks()+5000;
	app->game.hint_grab = 0;
	app->game.hint_give = 0;
	app->game.hint_build = 0;
	app->game.board.zombie_memory1 = 0;
	app->game.board.zombie_memory2 = 0;

	/**
	 * Player 1 init settings
	 * */
	Body *p1body = &app->game.player1.body;
	p1body->status = BODY_DEAD;
	p1body->ang_vel = 0.04;
	p1body->max_vel = 4;
	p1body->angle = 0;
	p1body->life = 100.0;
	p1body->item.type = &app->game.itemtype[ITEM_PLAYER_BULLET];
	p1body->item.ammo_used = 0 ;
	p1body->shoot_key = SDLK_LSHIFT;
	p1body->last_ai = 0;

	/**
	 * Player 2 init settings
	 * */
	Body *p2body = &app->game.player2.body;
	p2body->status = BODY_DEAD;
	p2body->ang_vel = 0.4;
	p2body->max_vel = 6;
	p2body->angle = 1;
	p2body->life = 100.0;
	p2body->item.type = &app->game.itemtype[ITEM_PLAYER_BULLET];
	p2body->item.ammo_used = 0 ;
	p2body->shoot_key = SDLK_RSHIFT;
	p2body->last_ai = 0;

	setWave(app,0); // calls gameInit
}

void pauseOrJoinTheGame(App *app, Player *player){
  if(player->body.status == BODY_ALIVE){
	app->state = STATE_PAUSED;
	app->menu.selected = MENU_RESUME;
  } else{
	player->body.status = BODY_ALIVE;
	player->body.life = 100.0;
	player_spawn_pos(&app->game, &player->body.pos.x, &player->body.pos.y);
  }
}

void bindGameplayKeysDown(App *app, SDLKey *key){
	Player *player1 = &app->game.player1;
	Player *player2 = &app->game.player2;
	SDLMod mod = SDL_GetModState();

	switch(*key){
		case SDLK_1:
			pauseOrJoinTheGame(app, player1);
			break;
		case SDLK_2:
		case SDLK_0:
			pauseOrJoinTheGame(app, player2);
			break;
		case SDLK_6:
			setWave(app, (app->game.board.wave_count + app->game.board.wave_index + (mod & KMOD_SHIFT ? -1 : 1)) % app->game.board.wave_count);
			break;
		case SDLK_5:
			app->debug = (DEBUG_COUNT + app->debug + (mod & KMOD_SHIFT ? -1 : 1)) % DEBUG_COUNT;
			break;
		case SDLK_ESCAPE:
			app->state = STATE_PAUSED;
			app->menu.selected = MENU_RESUME;
			break;
		case SDLK_LCTRL:
		case SDLK_LALT:
			grab(app, &player1->body);
			break;
		case SDLK_RCTRL:
		case SDLK_RETURN:
		case SDLK_RALT:
			give(app, &player1->body, &player2->body);
			
			if(grab(app, &player2->body)
			&& player1->body.status==BODY_ALIVE
			&& app->game.hint_give == 0)
			{
				app->game.hint_give = SDL_GetTicks() + 5000;
			}
			break;
	}
}

void bindMenuKeysDown(App *app, SDLKey *key){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;
  Menu *menu = &app->menu;
  int firstMenu = MENU_NEW_GAME;

  /**
   * when game is paused there is more menu options
   * */
  if(app->state == STATE_PAUSED){
	firstMenu = MENU_RESUME;
  }

  switch(*key){
	case SDLK_UP:
	case SDLK_w:
	  if(menu->selected != firstMenu){
		menu->selected--;
	  }
	  break;
	case SDLK_DOWN:
	case SDLK_s:
	  if(menu->selected < MENU_COUNT - 1){
		menu->selected++;
	  }
	  break;
	case SDLK_1:
	case SDLK_2:
	case SDLK_0:
	  if(player2->body.status != BODY_ALIVE &&
		  player1->body.status != BODY_ALIVE) {
		  resetApp(app);
	  }
	  app->state = STATE_PLAYING;
	  if(*key != SDLK_1){
		  player2->body.status = BODY_ALIVE;
		  player2->body.life = 100;
		  player_spawn_pos(&app->game, &player2->body.pos.x, &player2->body.pos.y);
	  } else {
		  player1->body.status = BODY_ALIVE;
		  player1->body.life = 100;
		  player_spawn_pos(&app->game, &player1->body.pos.x, &player1->body.pos.y);
	  }
	  break;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
	case SDLK_LCTRL:
	case SDLK_RCTRL:
	case SDLK_LALT:
	case SDLK_RALT:
	case SDLK_RETURN:
	case SDLK_SPACE:
	  if(app->state == STATE_HELP){
		  app->state = app->stateBeforeCredits;
		  break;
	  }

	  if(app->state == STATE_CREDITS){
		if(app->credits == CREDITS_SOUND){
		  app->state = app->stateBeforeCredits;
		  app->credits = CREDITS_TEAM;
		} else{
		  app->credits = CREDITS_SOUND;
		}
		break;
	  }

	  if(menu->selected == MENU_NEW_GAME){
		resetApp(app);
		app->state = STATE_PLAYING;
	  } else if (menu->selected == MENU_QUIT){
		finishHim(app);
	  } else if(menu->selected == MENU_CREDITS){
		AppState s = app->state;
		app->stateBeforeCredits = s;
		app->state = STATE_CREDITS;
	  } else if(menu->selected == MENU_HELP){
		AppState s = app->state;
		app->stateBeforeCredits = s;
		app->state = STATE_HELP;
	  } else if(menu->selected == MENU_RESUME){
		app->state = STATE_PLAYING;
	  }
	  if(*key == SDLK_RSHIFT){
		player2->body.status = BODY_ALIVE;
		player2->body.life = 100;
		player_spawn_pos(&app->game, &player2->body.pos.x, &player2->body.pos.y);
	  } else {
		//printf("player 1 is ready\n");
		player1->body.status = BODY_ALIVE;
		player1->body.life = 100;
		player_spawn_pos(&app->game, &player1->body.pos.x, &player1->body.pos.y);
	  }
	  break;
	case SDLK_ESCAPE:
	  finishHim(app);
	  break;
  }
}

/**
 * Keystate is responsible to handle pressed keys
 */
void bindGameplayKeystate(App *app){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;

  Uint8 *keystate;
  keystate = SDL_GetKeyState(NULL);

  /**
   * Player 1 settings:
   * Q = UP; W = DOWN; E = LEFT; R = RIGHT
   * A = ATTACK
   * S = SECONDARY ATTACK
   * */

  player_move(app, &player1->body,
	  keystate[SDLK_w],
	  keystate[SDLK_d],
	  keystate[SDLK_s],
	  keystate[SDLK_a],
	  keystate[SDLK_LCTRL] || keystate[SDLK_LALT]
	  );

  /**
   * Player 2 settings:
   * T = UP; Y = DOWN; U = LEFT;I = RIGHT
   * Z = ATTACK
   * S = SECONDARY ATTACK
   * */
  player_move(app, &player2->body,
	  keystate[SDLK_UP],
	  keystate[SDLK_RIGHT],
	  keystate[SDLK_DOWN],
	  keystate[SDLK_LEFT],
	  keystate[SDLK_RCTRL] || keystate[SDLK_RALT] || keystate[SDLK_RETURN]
	  );

  if(keystate[SDLK_LSHIFT])
	shoot(app, &player1->body);
  if(keystate[SDLK_RSHIFT] ) {
	if(player2->body.item.type->build) {
		build(app, &player2->body);
	} else {
		shoot(app, &player2->body);
	}
  }
}

void bindKeyboard(App *app)
{
  SDL_Event event;
  while(SDL_PollEvent(&event)){
	switch(event.type) {
	  case SDL_KEYDOWN:
		if (app->state == STATE_PLAYING){
		  bindGameplayKeysDown(app, &event.key.keysym.sym);
		} else{
		  bindMenuKeysDown(app, &event.key.keysym.sym);
		}
	}
  }

  if (app->state == STATE_PLAYING){
	bindGameplayKeystate(app);
  }
}

void handleDelay(Uint32 start) {
  Uint32 end = SDL_GetTicks();
  int actual_delta = end - start;
  int expected_delta = 1000/FPS;
  int delay = MAX(0, expected_delta - actual_delta);
  //printf("delay %d %d %d\n", actual_delta, expected_delta, delay);
  SDL_Delay(delay);
}

void spawnEnemy(App *app)
{
	Game *game = &app->game;
	int x,y,i;
	Wave *wave = &game->board.wave[game->board.wave_index];
	int t = SDL_GetTicks();

	// printf("spawn tick %d\n", t);
	//if(game->on_screen_enemies>0 && t < app->game.spawnTime)
	if(t < app->game.spawnTime)
		return;
	// printf("spawn started %d\n", t);

	int spawnDelay = wave->enemy_spawn_interval;

	int roulette = 0;
	for(i=0;i<ENEMY_TYPE_COUNT;i++) {
		roulette += wave->enemy_chance[i];
	}

	int spawn = (int)ceil(wave->enemy_count_per_spawn/2.) 
		+ (rand() % (int)ceil(wave->enemy_count_per_spawn/2.));

	if(spawn + game->on_screen_enemies > wave->enemy_count_on_screen) {
		// printf("spawn %d on_screen\n", game->on_screen_enemies);
		spawn = wave->enemy_count_on_screen - game->on_screen_enemies;
	}
	
	if(spawn + game->total_enemies > wave->enemy_count) {
		// printf("spawn %d wave\n", game->total_enemies);
		spawn = wave->enemy_count - game->total_enemies;
	}

	// printf("spawn %d enemies\n", spawn);
	while(spawn > 0) {
		Enemy *enemy = NULL;
		for(i = 0; i < wave->enemy_count; i++)
		{
			if(game->enemies[i].body.status == BODY_DEAD)
			{
				game->total_enemies ++;
				game->on_screen_enemies ++;
				spawn --;
				enemy = &game->enemies[i];
				break;
			}
		}

		if(enemy != NULL && enemy_spawn_pos(game, &x,&y))
		{
			// printf("spawn %d\n", i);
			enemy->body.status = BODY_ALIVE;
			int i = 0;
			int k = rand() % roulette;
			for(i=0; i<ENEMY_TYPE_COUNT;i++) {
				int kk = wave->enemy_chance[i];
				// printf("type %d %d %d\n",i, k, kk);
				if(k<kk) break;
				k-=kk;
			}
			// printf("type selected %d %d\n",i, k);
			enemy->image = game->enemy_class[i].image;
			Body *enemybody = &enemy->body;
			enemybody->life = game->enemy_class[i].max_life;
			enemybody->item.type = game->enemy_class[i].type;
			enemybody->ang_vel = 0.05;
			enemybody->max_vel = game->enemy_class[i].vel;
			enemybody->angle = 0;
			enemybody->pos.x = x;
			enemybody->pos.y = y;
			enemybody->last_ai = 0;
			enemybody->exploded = 0;
		} else {
			spawnDelay = 500;
			break; // did not finished group spawn, persist
		}
	}
	app->game.spawnTime = t + spawnDelay;
}

void loadMap(App *app) {
  char image_path[256];
  char hit_path[256];
  char wave_path[256];
  char *map_name = "map1";
  sprintf(image_path, "data/%s.png", map_name);
  sprintf(hit_path, "data/%s_hit.png", map_name);
  // TODO sprintf(wave_path, "data/%s_wave.txt", map_name);
  app->game.board.base_image = IMG_Load(image_path);
  app->game.board.base_hit = IMG_Load(hit_path);

  app->game.board.image = SDL_CreateRGBSurface(SDL_HWSURFACE, mapWidth*tileSize, mapHeight*tileSize, 32, 0, 0, 0, 0);
  app->game.board.hit = SDL_CreateRGBSurface(SDL_SWSURFACE, mapWidth, mapHeight, 32, 0, 0, 0, 0);

  // TODO set other wave fields
  int bx = 44;
  int by = 32;
  app->game.board.wave_count=19;
  app->game.board.wave[0].x=bx*0;
  app->game.board.wave[0].y=by*0;
  app->game.board.wave[0].w=mapWidth;
  app->game.board.wave[0].h=mapHeight;
  app->game.board.wave[0].enemy_spawn_interval=5000;
  app->game.board.wave[0].enemy_count=10;
  app->game.board.wave[0].enemy_count_on_screen=3;
  app->game.board.wave[0].enemy_count_per_spawn=3;
  app->game.board.wave[0].enemy_chance[ENEMY_MEDIC]=1;
  app->game.board.wave[0].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[0].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[0].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[1].x=bx*1;
  app->game.board.wave[1].y=by*0;
  app->game.board.wave[1].w=mapWidth;
  app->game.board.wave[1].h=mapHeight;
  app->game.board.wave[1].enemy_spawn_interval=5000;
  app->game.board.wave[1].enemy_count=40;
  app->game.board.wave[1].enemy_count_on_screen=20;
  app->game.board.wave[1].enemy_count_per_spawn=10;
  app->game.board.wave[1].enemy_chance[ENEMY_MEDIC]=1;
  app->game.board.wave[1].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[1].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[1].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[2].x=bx*2;
  app->game.board.wave[2].y=by*0;
  app->game.board.wave[2].w=mapWidth;
  app->game.board.wave[2].h=mapHeight;
  app->game.board.wave[2].enemy_spawn_interval=2000;
  app->game.board.wave[2].enemy_count=120;
  app->game.board.wave[2].enemy_count_on_screen=80;
  app->game.board.wave[2].enemy_count_per_spawn=10;
  app->game.board.wave[2].enemy_chance[ENEMY_MEDIC]=1;
  app->game.board.wave[2].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[2].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[2].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[3].y=by*0;
  app->game.board.wave[3].x=bx*3-2;
  app->game.board.wave[3].w=(mapWidth-5);
  app->game.board.wave[3].h=mapHeight;
  app->game.board.wave[3].enemy_spawn_interval=250;
  app->game.board.wave[3].enemy_count=120;
  app->game.board.wave[3].enemy_count_on_screen=30;
  app->game.board.wave[3].enemy_count_per_spawn=2;
  app->game.board.wave[3].enemy_chance[ENEMY_MEDIC]=1;
  app->game.board.wave[3].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[3].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[3].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[4].x=bx*3-6;
  app->game.board.wave[4].y=by*1;
  app->game.board.wave[4].w=mapWidth-1;
  app->game.board.wave[4].h=mapHeight;
  app->game.board.wave[4].enemy_spawn_interval=2000;
  app->game.board.wave[4].enemy_count=250;
  app->game.board.wave[4].enemy_count_on_screen=80;
  app->game.board.wave[4].enemy_count_per_spawn=20;
  app->game.board.wave[4].enemy_chance[ENEMY_MEDIC]=1;
  app->game.board.wave[4].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[4].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[4].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[5].x=bx*2;
  app->game.board.wave[5].y=by*1;
  app->game.board.wave[5].w=mapWidth;
  app->game.board.wave[5].h=mapHeight;
  app->game.board.wave[5].enemy_spawn_interval=6000;
  app->game.board.wave[5].enemy_count=100;
  app->game.board.wave[5].enemy_count_on_screen=30;
  app->game.board.wave[5].enemy_count_per_spawn=20;
  app->game.board.wave[5].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[5].enemy_chance[ENEMY_SOLDIER]=1;
  app->game.board.wave[5].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[5].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[6].x=bx*1;
  app->game.board.wave[6].y=by*1;
  app->game.board.wave[6].w=mapWidth;
  app->game.board.wave[6].h=mapHeight;
  app->game.board.wave[6].enemy_spawn_interval=7000;
  app->game.board.wave[6].enemy_count=110;
  app->game.board.wave[6].enemy_count_on_screen=40;
  app->game.board.wave[6].enemy_count_per_spawn=20;
  app->game.board.wave[6].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[6].enemy_chance[ENEMY_SOLDIER]=2;
  app->game.board.wave[6].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[6].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[7].x=bx*0;
  app->game.board.wave[7].y=by*1;
  app->game.board.wave[7].w=mapWidth;
  app->game.board.wave[7].h=mapHeight;
  app->game.board.wave[7].enemy_spawn_interval=8000;
  app->game.board.wave[7].enemy_count=120;
  app->game.board.wave[7].enemy_count_on_screen=45;
  app->game.board.wave[7].enemy_count_per_spawn=21;
  app->game.board.wave[7].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[7].enemy_chance[ENEMY_SOLDIER]=3;
  app->game.board.wave[7].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[7].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[8].x=bx*0;
  app->game.board.wave[8].y=by*2;
  app->game.board.wave[8].w=mapWidth;
  app->game.board.wave[8].h=mapHeight;
  app->game.board.wave[8].enemy_spawn_interval=8500;
  app->game.board.wave[8].enemy_count=130;
  app->game.board.wave[8].enemy_count_on_screen=50;
  app->game.board.wave[8].enemy_count_per_spawn=22;
  app->game.board.wave[8].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[8].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[8].enemy_chance[ENEMY_FASTER]=1;
  app->game.board.wave[8].enemy_chance[ENEMY_SUICIDAL]=0;

  app->game.board.wave[9].x=bx*0;
  app->game.board.wave[9].y=by*3+3;
  app->game.board.wave[9].w=(mapWidth-1);
  app->game.board.wave[9].h=(mapHeight-3);
  app->game.board.wave[9].enemy_spawn_interval=9000;
  app->game.board.wave[9].enemy_count=140;
  app->game.board.wave[9].enemy_count_on_screen=55;
  app->game.board.wave[9].enemy_count_per_spawn=23;
  app->game.board.wave[9].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[9].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[9].enemy_chance[ENEMY_FASTER]=0;
  app->game.board.wave[9].enemy_chance[ENEMY_SUICIDAL]=8;

  app->game.board.wave[10].x=bx*1;
  app->game.board.wave[10].y=by*3;
  app->game.board.wave[10].w=mapWidth;
  app->game.board.wave[10].h=mapHeight;
  app->game.board.wave[10].enemy_spawn_interval=5000;
  app->game.board.wave[10].enemy_count=300;
  app->game.board.wave[10].enemy_count_on_screen=75;
  app->game.board.wave[10].enemy_count_per_spawn=30;
  app->game.board.wave[10].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[10].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[10].enemy_chance[ENEMY_FASTER]=1;
  app->game.board.wave[10].enemy_chance[ENEMY_SUICIDAL]=1;

  app->game.board.wave[11].x=bx*2;
  app->game.board.wave[11].y=by*3+3;
  app->game.board.wave[11].w=(mapWidth-4);
  app->game.board.wave[11].h=(mapHeight-3);
  app->game.board.wave[11].enemy_spawn_interval=5000;
  app->game.board.wave[11].enemy_count=300;
  app->game.board.wave[11].enemy_count_on_screen=30;
  app->game.board.wave[11].enemy_count_per_spawn=10;
  app->game.board.wave[11].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[11].enemy_chance[ENEMY_SOLDIER]=0;
  app->game.board.wave[11].enemy_chance[ENEMY_FASTER]=5;
  app->game.board.wave[11].enemy_chance[ENEMY_SUICIDAL]=2;

  app->game.board.wave[12].x=bx*2;
  app->game.board.wave[12].y=by*2;
  app->game.board.wave[12].w=mapWidth;
  app->game.board.wave[12].h=mapHeight;
  app->game.board.wave[12].enemy_spawn_interval=11000;
  app->game.board.wave[12].enemy_count=170;
  app->game.board.wave[12].enemy_count_on_screen=70;
  app->game.board.wave[12].enemy_count_per_spawn=26;
  app->game.board.wave[12].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[12].enemy_chance[ENEMY_SOLDIER]=4;
  app->game.board.wave[12].enemy_chance[ENEMY_FASTER]=6;
  app->game.board.wave[12].enemy_chance[ENEMY_SUICIDAL]=3;

  app->game.board.wave[13].x=bx*3;
  app->game.board.wave[13].y=by*2;
  app->game.board.wave[13].w=(mapWidth-5);
  app->game.board.wave[13].h=mapHeight;
  app->game.board.wave[13].enemy_spawn_interval=11500;
  app->game.board.wave[13].enemy_count=170;
  app->game.board.wave[13].enemy_count_on_screen=75;
  app->game.board.wave[13].enemy_count_per_spawn=27;
  app->game.board.wave[13].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[13].enemy_chance[ENEMY_SOLDIER]=2;
  app->game.board.wave[13].enemy_chance[ENEMY_FASTER]=7;
  app->game.board.wave[13].enemy_chance[ENEMY_SUICIDAL]=4;

  app->game.board.wave[14].x=bx*3;
  app->game.board.wave[14].y=by*3+3;
  app->game.board.wave[14].w=mapWidth;
  app->game.board.wave[14].h=(mapHeight-3);
  app->game.board.wave[14].enemy_spawn_interval=12000;
  app->game.board.wave[14].enemy_count=180;
  app->game.board.wave[14].enemy_count_on_screen=80;
  app->game.board.wave[14].enemy_count_per_spawn=28;
  app->game.board.wave[14].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[14].enemy_chance[ENEMY_SOLDIER]=10;
  app->game.board.wave[14].enemy_chance[ENEMY_FASTER]=8;
  app->game.board.wave[14].enemy_chance[ENEMY_SUICIDAL]=5;

  app->game.board.wave[15].x=bx*4;
  app->game.board.wave[15].y=by*3+3;
  app->game.board.wave[15].w=mapWidth;
  app->game.board.wave[15].h=mapHeight;
  app->game.board.wave[15].enemy_spawn_interval=7000;
  app->game.board.wave[15].enemy_count=300;
  app->game.board.wave[15].enemy_count_on_screen=200;
  app->game.board.wave[15].enemy_count_per_spawn=100;
  app->game.board.wave[15].enemy_chance[ENEMY_MEDIC]=0;
  app->game.board.wave[15].enemy_chance[ENEMY_SOLDIER]=5;
  app->game.board.wave[15].enemy_chance[ENEMY_FASTER]=100;
  app->game.board.wave[15].enemy_chance[ENEMY_SUICIDAL]=1;

  app->game.board.wave[16].x=bx*4;
  app->game.board.wave[16].y=by*2;
  app->game.board.wave[16].w=mapWidth;
  app->game.board.wave[16].h=mapHeight;
  app->game.board.wave[16].enemy_spawn_interval=13000;
  app->game.board.wave[16].enemy_count=230;
  app->game.board.wave[16].enemy_count_on_screen=90;
  app->game.board.wave[16].enemy_count_per_spawn=30;
  app->game.board.wave[16].enemy_chance[ENEMY_MEDIC]=20;
  app->game.board.wave[16].enemy_chance[ENEMY_SOLDIER]=15;
  app->game.board.wave[16].enemy_chance[ENEMY_FASTER]=10;
  app->game.board.wave[16].enemy_chance[ENEMY_SUICIDAL]=2;

  app->game.board.wave[17].x=bx*4;
  app->game.board.wave[17].y=by*1;
  app->game.board.wave[17].w=mapWidth;
  app->game.board.wave[17].h=mapHeight;
  app->game.board.wave[17].enemy_spawn_interval=13500;
  app->game.board.wave[17].enemy_count=250;
  app->game.board.wave[17].enemy_count_on_screen=95;
  app->game.board.wave[17].enemy_count_per_spawn=32;
  app->game.board.wave[17].enemy_chance[ENEMY_MEDIC]=10;
  app->game.board.wave[17].enemy_chance[ENEMY_SOLDIER]=30;
  app->game.board.wave[17].enemy_chance[ENEMY_FASTER]=10;
  app->game.board.wave[17].enemy_chance[ENEMY_SUICIDAL]=12;

  app->game.board.wave[18].x=bx*4;
  app->game.board.wave[18].y=by*0+1;
  app->game.board.wave[18].w=mapWidth;
  app->game.board.wave[18].h=mapHeight;
  app->game.board.wave[18].enemy_spawn_interval=15000;
  app->game.board.wave[18].enemy_count=ENEMY_COUNT;
  app->game.board.wave[18].enemy_count_on_screen=400;
  app->game.board.wave[18].enemy_count_per_spawn=50;
  app->game.board.wave[18].enemy_chance[ENEMY_MEDIC]=20;
  app->game.board.wave[18].enemy_chance[ENEMY_SOLDIER]=40;
  app->game.board.wave[18].enemy_chance[ENEMY_FASTER]=30;
  app->game.board.wave[18].enemy_chance[ENEMY_SUICIDAL]=10;

}

void gameEnding(App *app);
void setWave(App *app, int wave_index) {
	if(wave_index >= app->game.board.wave_count) {
		gameEnding(app);
		return;
	}
	app->game.board.wave_index = wave_index;
	app->game.board.wave_start = SDL_GetTicks();
	app->game.total_enemies = 0;
	app->game.on_screen_enemies = 0;

	{ // cut map slice
		SDL_FillRect(app->game.board.hit, NULL, 0);
		SDL_Rect rect = {
			app->game.board.wave[wave_index].x*tileSize,
			app->game.board.wave[wave_index].y*tileSize,
			mapWidth*tileSize,
			mapHeight*tileSize
		};
		SDL_BlitSurface(app->game.board.base_image, &rect, app->game.board.image, NULL);
	}

	{ // cut hit map slice
		SDL_FillRect(app->game.board.hit, NULL, 0);
		SDL_Rect rect = {
			app->game.board.wave[wave_index].x,
			app->game.board.wave[wave_index].y,
			app->game.board.wave[wave_index].w,
			app->game.board.wave[wave_index].h
		};
		SDL_BlitSurface(app->game.board.base_hit, &rect, app->game.board.hit, NULL);
	}


	moveInit(app);
	gameInit(app);
}

void loadItems(App *app) {
	app->game.itemtype[ITEM_ENEMY_MEDIC].damage = 1;
	app->game.itemtype[ITEM_ENEMY_MEDIC].range = 2*tileSize;
	app->game.itemtype[ITEM_ENEMY_MEDIC].score = 1;
	app->game.itemtype[ITEM_ENEMY_MEDIC].hit_image = IMG_Load("data/bullet_hit.png");
	app->game.itemtype[ITEM_ENEMY_MEDIC].sound = Mix_LoadWAV("sounds/ouch.wav");

	app->game.itemtype[ITEM_ENEMY_SOLDIER].damage = 4;
	app->game.itemtype[ITEM_ENEMY_SOLDIER].range = 2*tileSize;
	app->game.itemtype[ITEM_ENEMY_SOLDIER].score = 5;
	app->game.itemtype[ITEM_ENEMY_SOLDIER].hit_image = IMG_Load("data/bullet_hit.png");
	app->game.itemtype[ITEM_ENEMY_SOLDIER].sound = Mix_LoadWAV("sounds/ouch.wav");

	app->game.itemtype[ITEM_ENEMY_FASTER].damage = 2;
	app->game.itemtype[ITEM_ENEMY_FASTER].range = 1*tileSize;
	app->game.itemtype[ITEM_ENEMY_FASTER].score = 2;
	app->game.itemtype[ITEM_ENEMY_FASTER].hit_image = IMG_Load("data/bullet_hit.png");
	app->game.itemtype[ITEM_ENEMY_FASTER].sound = Mix_LoadWAV("sounds/ouch.wav");

	app->game.itemtype[ITEM_ENEMY_SUICIDAL].damage = 20;
	app->game.itemtype[ITEM_ENEMY_SUICIDAL].range = 3*tileSize;
	app->game.itemtype[ITEM_ENEMY_SUICIDAL].score = 3;
	app->game.itemtype[ITEM_ENEMY_SUICIDAL].hit_image = IMG_Load("data/fire_hit.png");
	app->game.itemtype[ITEM_ENEMY_SUICIDAL].sound = Mix_LoadWAV("sounds/ouch.wav");
	app->game.itemtype[ITEM_ENEMY_SUICIDAL].should_explode = 1;

	app->game.itemtype[ITEM_PLAYER_BULLET].chance = 50;
	app->game.itemtype[ITEM_PLAYER_BULLET].damage = 15;
	app->game.itemtype[ITEM_PLAYER_BULLET].range = 1024;
	app->game.itemtype[ITEM_PLAYER_BULLET].hit_image = IMG_Load("data/bullet_hit.png");
	app->game.itemtype[ITEM_PLAYER_BULLET].image = IMG_Load("data/bullet_ammo.png");
	app->game.itemtype[ITEM_PLAYER_BULLET].shot_image = IMG_Load("data/bullet_shot.png");
	app->game.itemtype[ITEM_PLAYER_BULLET].freq = 8;
	app->game.itemtype[ITEM_PLAYER_BULLET].spread = 3;
	app->game.itemtype[ITEM_PLAYER_BULLET].ammo_total = 1000;
	app->game.itemtype[ITEM_PLAYER_BULLET].sound = Mix_LoadWAV("sounds/machinegun.wav");

	app->game.itemtype[ITEM_PLAYER_FLAME].chance = 5;
	app->game.itemtype[ITEM_PLAYER_FLAME].damage = 100;
	app->game.itemtype[ITEM_PLAYER_FLAME].range = 150;
	app->game.itemtype[ITEM_PLAYER_FLAME].image = IMG_Load("data/fire_ammo.png");
	app->game.itemtype[ITEM_PLAYER_FLAME].hit_image = IMG_Load("data/fire_hit.png");
	app->game.itemtype[ITEM_PLAYER_FLAME].shot_image = IMG_Load("data/fire_shot.png");
	app->game.itemtype[ITEM_PLAYER_FLAME].spread = 20;
	app->game.itemtype[ITEM_PLAYER_FLAME].sound = Mix_LoadWAV("sounds/flame.wav");
	app->game.itemtype[ITEM_PLAYER_FLAME].freq = 2;
	app->game.itemtype[ITEM_PLAYER_FLAME].ammo_total = 250;

	app->game.itemtype[ITEM_HEALTH_PACK].chance = 20;
	app->game.itemtype[ITEM_HEALTH_PACK].damage = -50;
	app->game.itemtype[ITEM_HEALTH_PACK].image = IMG_Load("data/health.png");

	app->game.itemtype[ITEM_BUILD].build= 1;
	app->game.itemtype[ITEM_BUILD].damage = 0;
	app->game.itemtype[ITEM_BUILD].freq= 0;
	app->game.itemtype[ITEM_BUILD].sound = Mix_LoadWAV("sounds/shovel.wav");
	app->game.itemtype[ITEM_BUILD].image = IMG_Load("data/brick_ammo.png");
	app->game.itemtype[ITEM_BUILD].hit_image = IMG_Load("data/brick.png");

}

void loadEnemies(App *app) {
  app->game.enemy_class[ENEMY_MEDIC].image = IMG_Load("data/zombie2.png");
  app->game.enemy_class[ENEMY_MEDIC].type = &app->game.itemtype[ITEM_ENEMY_MEDIC];
  app->game.enemy_class[ENEMY_MEDIC].max_life = 100;
  app->game.enemy_class[ENEMY_MEDIC].vel = 4;

  app->game.enemy_class[ENEMY_FASTER].image = IMG_Load("data/faster-front.png");
  app->game.enemy_class[ENEMY_FASTER].type = &app->game.itemtype[ITEM_ENEMY_MEDIC];
  app->game.enemy_class[ENEMY_FASTER].max_life = 20;
  app->game.enemy_class[ENEMY_FASTER].vel = 10;

  app->game.enemy_class[ENEMY_SUICIDAL].image = IMG_Load("data/suicidal-front.png");
  app->game.enemy_class[ENEMY_SUICIDAL].type = &app->game.itemtype[ITEM_ENEMY_SUICIDAL];
  app->game.enemy_class[ENEMY_SUICIDAL].max_life = 120;
  app->game.enemy_class[ENEMY_SUICIDAL].vel = 6;

  app->game.enemy_class[ENEMY_SOLDIER].image = IMG_Load("data/zombie1.png");
  app->game.enemy_class[ENEMY_SOLDIER].type = &app->game.itemtype[ITEM_ENEMY_SOLDIER];
  app->game.enemy_class[ENEMY_SOLDIER].max_life = 500;
  app->game.enemy_class[ENEMY_SOLDIER].vel = 3;
}


int grab(App *app, Body *body)
{
	int x = body->pos.x/tileSize;
	int y = body->pos.y/tileSize;
	int dx,dy;
	int got = 0;

	if(body->status != BODY_ALIVE)
		return 0;

	/*
	               13(-1,-2) 09(+0,-2) 14(+1,-2)
	     20(-2,-1) 08(-1,-1) 04(+0,-1) 05(+1,-1) 15(+2,-1) 
	     12(-2,+0) 03(-1,+0) 00(+0,+0) 01(+1,+0) 10(+2,+0) 
	     19(-2,+1) 07(-1,+1) 02(+0,+1) 06(+1,+1) 16(+2,+1) 
	               18(-1,+2) 11(+0,+2) 17(+1,+2)
	*/
	int search[21][2] = {
		{+0,+0},
		{+1,+0},
		{+0,+1},
		{-1,+0},
		{+0,-1},
		{+1,-1},
		{+1,+1},
		{-1,+1},
		{-1,-1},
		{+0,-2},
		{+2,+0},
		{+0,+2},
		{-2,+0},
		{-1,-2},
		{+1,-2},
		{+2,-1},
		{+2,+1},
		{+1,+2},
		{-1,+2},
		{-2,+1},
		{-2,-1}
	};

	int i,idx = 0;
	for(i=0;i<21;i++) {
		int xx = x+search[i][0];
		int yy = y+search[i][1];
		//printf("x %d y %d dx %d dy %d\n", xx, yy, search[i][0], search[i][1]);
		if(xx<0||xx>=mapWidth||yy<0||yy>=mapHeight) continue;
		idx = app->game.board.powerup[xx][yy];
		if(idx) break;
	}

	if(idx && app->game.board.powerups[--idx].should_show) {
		if(app->game.board.powerups[idx].type->damage < 0) {
			body->life -= app->game.board.powerups[idx].type->damage;
			if(body->life > 100.0){
				body->life = 100.0;
			}
		} else  {
			body->item = app->game.board.powerups[idx];
			got = 1;
		}
		app->game.board.powerups[idx].should_show = 0;
		app->game.board.powerup[x][y] = 0;
	}
	return got;
}
int give(App *app, Body *body1, Body *body2)
{
	if(body1->status != BODY_ALIVE
	|| body2->status != BODY_ALIVE)
		return 0;
	int x1 = body1->pos.x/tileSize;
	int y1 = body1->pos.y/tileSize;
	int x2 = body2->pos.x/tileSize;
	int y2 = body2->pos.y/tileSize;

	if(body2->item.type->freq != 0){
		if(fabs(x1 - x2) < 5 && fabs(y1 - y2) < 5){
			body1->item = body2->item;
			body2->item.type = &app->game.itemtype[ITEM_BUILD];
			body2->item.ammo_used = 0 ;
			if(app->game.hint_build == 0) {
				app->game.hint_build = SDL_GetTicks() + 5000;
			}
			return 1;
		}
	}
	return 0;
}

int hit(App *app, Body *source, Body *target){
	if(target == NULL || source == NULL || source->item.type == NULL) {
		// FIXME trying to understand what leads to segfault here
		printf("target %p source %p\n", target, source);
		printf("source->item.type %p\n", source->item.type);
		return 0;
	}

	float dist = sqrt(
		pow(target->pos.x - source->pos.x,2)+
		pow(target->pos.y - source->pos.y,2)
	);
	if(dist > source->item.type->range) {
		//printf("dist %f > source->item.type->range %d dmg %d\n", dist, source->item.type->range, source->item.type->damage);
		return 0;
	}

	target->life -= source->item.type->damage;

	if(source->item.type->hit_image) {
		SDL_Rect rect = {
			target->pos.x - source->item.type->hit_image->w/2,
			target->pos.y - source->item.type->hit_image->h/2,
			source->item.type->hit_image->w,
			source->item.type->hit_image->h
		};
		SDL_BlitSurface(source->item.type->hit_image, NULL, app->screen, &rect);
		// printf("splash %d %d\n", target->pos.x, target->pos.y);
	}

	if(source->item.type->should_explode && source != target) {
		source->life = 1;
		source->exploded = 1;
		hit(app, source, source);
	}

	if(target->status != BODY_DEAD){
		playSound(target->onHitSound);
	}

	if(target->life <= 0){
		target->life = 0;
		if(target->status == BODY_ALIVE){
			int enemy_killed = !!target->item.type->score; // enemy killed
			if(!app->debug || enemy_killed) { // player immortal on debug
				int x0 = target->pos.x/tileSize;
				int y0 = target->pos.y/tileSize;
				int search[21][2] = {
					{+0,+0},
					{+1,+0},
					{+0,+1},
					{-1,+0},
					{+0,-1},
					{+1,-1},
					{+1,+1},
					{-1,+1},
					{-1,-1},
					{+0,-2},
					{+2,+0},
					{+0,+2},
					{-2,+0},
					{-1,-2},
					{+1,-2},
					{+2,-1},
					{+2,+1},
					{+1,+2},
					{-1,+2},
					{-2,+1},
					{-2,-1}
				};

				int i;
				Wave *wave = &app->game.board.wave[app->game.board.wave_index];
				if(enemy_killed) {
					app->game.on_screen_enemies --;
					if(target->exploded) {
						app->game.total_enemies --;
					} else {
						app->game.kill_count ++;
						app->game.total_kill_count ++;
						int score = target->item.type->score;
						for(i=0;i<21;i++) {
							int x = x0+search[i][0];
							int y = y0+search[i][1];
							if( x < 1 || y < 1 || x >= wave->w-1 || y >= wave->h-1 || walkability[x][y]==1) 
								continue; // dont outside or on the enemy spawn borders
							int s = score * ceil( (21-i) / 4.);
							app->game.board.death1[x][y] += s;
							app->game.board.death2[x][y] += s;
						}
					}
				} else { // player_killed
					int x1 = source->pos.x/tileSize;
					int y1 = source->pos.y/tileSize;
					for(i=0;i<21;i++) {
						int x = x1+search[i][0];
						int y = y1+search[i][1];
						if( x < 1 || y < 1 || x >= wave->w-1 || y >= wave->h-1 || walkability[x][y]==1) 
							continue; // dont outside or on the enemy spawn borders
						app->game.board.death1[x][y] /= 2;
					}
					for(i=0;i<21;i++) {
						int x = x0+search[i][0];
						int y = y0+search[i][1];
						if( x < 1 || y < 1 || x >= wave->w-1 || y >= wave->h-1 || walkability[x][y]==1) 
							continue; // dont outside or on the enemy spawn borders
						app->game.board.death1[x][y] /= 2;
					}
				}
				target->status = BODY_DEAD;
				int index = app->game.board.wave_index;
				int count = app->game.board.wave[app->game.board.wave_index].enemy_count;
				if(count == app->game.kill_count){
					setWave(app, index+1);    
				}
			}
		}
		return 1;
	}
	return 0;
}

inline int is_air(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;
	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		return 1;
	return game->board.hittable[x][y];
}

inline int draw(App *app, Body *body, int x, int y)
{
	int i;

	if(x >= 0 && x < app->screen->w && y >= 0 && y < app->screen->h) {
#if 0
		Uint8 *p = ((Uint8*)app->screen->pixels) + (x*app->screen->format->BytesPerPixel+y*app->screen->pitch);
		p[0] = 0xff;
		p[1] = 0xff;
#else
		if(body->item.type->shot_image && (rand() % (body->item.type->freq)) == 0) {
			x += (rand() % (body->item.type->spread/2+1)) - body->item.type->spread/4;
			y += (rand() % (body->item.type->spread/2+1)) - body->item.type->spread/4;
			SDL_Rect rect = {
				x - body->item.type->shot_image->w/2,
				y - body->item.type->shot_image->h/2,
				body->item.type->shot_image->w,
				body->item.type->shot_image->h
			};
			SDL_BlitSurface(body->item.type->shot_image, NULL, app->screen, &rect);
		}
#endif
	}

	int target = 0;
	for(i=0; i<enemyTileHeight; i++) { // hit the whole height of the enemies, not just the feet
		// printf("hit %d,%d-%d /%d\n", x,y,i *tileSize, enemyTileHeight);
		int tg = is_air(&app->game, body, x, y+i*tileSize);
		if(tg >=4 || tg && i==0) {
			// printf("i %d tg %d\n", i, tg);
			target = tg;

			// hit_built
			x/=tileSize;
			y/=tileSize;
			if(i==0
				&& x>=0 && y>=0 && x<mapWidth && y<mapHeight 
				&& app->game.board.built[x][y] > 0)
			{
				app->game.board.built[x][y]/=2;
			}
			break;
		}
	}

	
	if(target>=4) {
		int idx = target - 4;
		hit(app, body, &app->game.enemies[idx].body);
	}

	return target;
}

inline int lasersight(App *app, Body *body, int x, int y, int n)
{
	int hit = 0;
	if(x >= 0 && x < app->screen->w && y >= 0 && y < app->screen->h) {
		Uint32 *p = (Uint32*)(((Uint8*)app->screen->pixels) + (x*app->screen->format->BytesPerPixel+y*app->screen->pitch));
		hit = is_air(&app->game, body, x, y);
		if(hit) n=0;
		if(n<=0x80) {
			Uint32 color = SDL_MapRGB(app->screen->format, 0xff-n, 0,0 );
			*p = color;
		}
	}

  return hit;
}

int aim(App *app, Body *body)
{
  Uint8 *keystate;
  keystate = SDL_GetKeyState(NULL);
  if(keystate[body->shoot_key] ) return;
	int x1, y1, x2, y2;
	int dx, dy, i, e;
	int incx, incy, inc1, inc2;
	int x,y;
	int range;
	int n=0;
	if(body->status != BODY_ALIVE)
		return;

	range = app->screen->w;

	x1 = body->pos.x;
	y1 = body->pos.y;

	float a = (int)(body->angle)%360;
	x2 = x1 + cos(a * M_PI / 180.) * range;
	y2 = y1 - sin(a * M_PI / 180.) * range;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;
	incx = 1;
	if(x2 < x1) incx = -1;
	incy = 1;
	if(y2 < y1) incy = -1;
	x=x1;
	y=y1;

	if(dx > dy)
	{
	  if(lasersight(app,body,x,y,n++)) return 1;
      e = 2*dy - dx;
      inc1 = 2*( dy -dx);
      inc2 = 2*dy;
      for(i = 0; i < dx; i++)
      {
        if(e >= 0)
        {
          y += incy;
          e += inc1;
        } else e += inc2;
        x += incx;
        if(lasersight(app,body,x,y,n++)) return 1;
      }
  }
  else
  {
    if(lasersight(app,body,x,y,n++)) return 1;
      e = 2*dx - dy;
      inc1 = 2*( dx - dy);
      inc2 = 2*dx;
      for(i = 0; i < dy; i++)
      {
        if(e >= 0)
        {
          x += incx;
          e += inc1;
        }
        else e += inc2;
        y += incy;
        if(lasersight(app,body,x,y,n++)) return 1;
      }
  }
  return 0;
}

int shoot(App *app, Body *body)
{
	int x1, y1, x2, y2;
	int dx, dy, i, e;
	int incx, incy, inc1, inc2;
	int x,y;
	int range;
	if(body->status != BODY_ALIVE)
		return;

	if((body->item.ammo_used >= body->item.type->ammo_total) && !app->debug){
		return;
	}

	++body->item.ammo_used;
	range = body->item.type->range;
	playSound(body->item.type->sound, -1);

	x1 = body->pos.x;
	y1 = body->pos.y;

	float a = (int)(body->angle + ((rand() % (body->item.type->spread+1)) - body->item.type->spread/2))%360;
	x2 = x1 + cos(a * M_PI / 180.) * range;
	y2 = y1 - sin(a * M_PI / 180.) * range;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;
	incx = 1;
	if(x2 < x1) incx = -1;
	incy = 1;
	if(y2 < y1) incy = -1;
	x=x1;
	y=y1;

	if(dx > dy)
	{
	  if(draw(app,body,x,y)) return 1;
      e = 2*dy - dx;
      inc1 = 2*( dy -dx);
      inc2 = 2*dy;
      for(i = 0; i < dx; i++)
      {
        if(e >= 0)
        {
          y += incy;
          e += inc1;
        } else e += inc2;
        x += incx;
        if(draw(app,body,x,y)) return 1;
      }
  }
  else
  {
    if(draw(app,body,x,y)) return 1;
      e = 2*dx - dy;
      inc1 = 2*( dx - dy);
      inc2 = 2*dx;
      for(i = 0; i < dy; i++)
      {
        if(e >= 0)
        {
          x += incx;
          e += inc1;
        }
        else e += inc2;
        y += incy;
        if(draw(app,body,x,y)) return 1;
      }
  }

  return 0;
}

int build(App *app, Body *body)
{
	int x1, y1, x2, y2;
	int i;
	int range;
	if(body->status != BODY_ALIVE)
		return;

	range = tileSize*3;
	playSound(body->item.type->sound, -1);

	x1 = body->pos.x;
	y1 = body->pos.y;

	float a = (int)(body->angle + ((rand() % (body->item.type->spread+1)) - body->item.type->spread/2))%360;
	x2 = x1 + cos(a * M_PI / 180.) * range;
	y2 = y1 - sin(a * M_PI / 180.) * range;

	x2/=tileSize;
	y2/=tileSize;

	if(x2<0) x2=0;
	if(x2>mapWidth-1) x2=mapWidth-1;
	if(y2<0) y2=0;
	if(y2>mapHeight-1) y2=mapHeight-1;

	int search[21][2] = {
		{+0,+0},
		{+1,+0},
		{+0,+1},
		{-1,+0},
		{+0,-1},
		{+1,-1},
		{+1,+1},
		{-1,+1},
		{-1,-1},
		{+0,-2},
		{+2,+0},
		{+0,+2},
		{-2,+0},
		{-1,-2},
		{+1,-2},
		{+2,-1},
		{+2,+1},
		{+1,+2},
		{-1,+2},
		{-2,+1},
		{-2,-1}
	};

	Wave *wave = &app->game.board.wave[app->game.board.wave_index];
	for(i=0;i<21;i++) {
		int x = x2+search[i][0];
		int y = y2+search[i][1];
		if( x < 1 || y < 1 || x >= wave->w-1 || y >= wave->h-1) 
			continue; // dont build outside or on the enemy spawn borders
		if(app->game.board.built[x][y] || app->game.board.crowd[x][y]==0) {
			app->game.board.built[x][y]+=ceil((21-i)/4.);
			if(app->game.board.built[x][y] > BUILD_LIMIT)
				app->game.board.built[x][y] = BUILD_LIMIT;
		}
	}

}


void addPowerup(App *app)
{
	int i,j;
	int x,y;

	int t = SDL_GetTicks();
	if(t < app->game.spawnPowerupTime)
		return;
	app->game.spawnPowerupTime = t + 10000;

	int roulette = 0;
	for(i=0; i < ITEM_PLAYER_COUNT; i++) {
		roulette += app->game.itemtype[i].chance;
	}


	if(powerup_spawn_pos(&app->game, &x, &y)) {
		for(i=0; i < POWERUP_COUNT; i++) {
			if(app->game.board.powerups[i].should_show == 0) {
				int k = rand() % roulette;
				for(j=0;j<ITEM_PLAYER_COUNT;j++) {
					int kk = app->game.itemtype[j].chance;
					//printf("type %d %d/%d/%d\n",j, k, kk, roulette);
					if(k<kk) break;
					k-=kk;
				}
				app->game.board.powerups[i].should_show = 1;
				app->game.board.powerups[i].ammo_used = 0;
				app->game.board.powerups[i].type = &app->game.itemtype[j];
				app->game.board.powerups[i].x = x;
				app->game.board.powerups[i].y = y;
				app->game.board.powerup[x/tileSize][y/tileSize] = 1+i;
				if(app->game.hint_grab == 0) {
					app->game.hint_grab = t + 5000;
				}
				break;
			}
		}
	}
}

void gameEnding(App *app)
{
	int i;
	playMusic("menu.mp3", -1);
	app->game.hint_pivot = -9999;
	app->game.hint_grab = -9999;
	app->game.hint_give = -9999;
	app->game.hint_build = -9999;
	app->game.player1.body.angle = 180;
	app->game.player2.body.angle = 180;
	for(;;) {
		float duration = pow(1.1,app->game.board.wave_index)/2.;
		int frames = duration*FPS;
		for(i=0; i<frames; i++) {
			Uint32 startTime = SDL_GetTicks();
			SDL_Event event;
			while(SDL_PollEvent(&event)){
				// slurp
			}
			renderStart(app);
			app->game.spawnPowerupTime = 0;
			addPowerup(app);
			renderFinish(app);
			handleDelay(startTime);
		}
		if( app->game.board.wave_index == 0) break;;
		setWave(app, app->game.board.wave_index-1);
		app->game.board.wave_start=-9999;
		app->game.kill_count = app->game.board.wave[app->game.board.wave_index].enemy_count;
	}
	app->state = STATE_GAMEOVER;
	app->game.won = 1;
}

int main(int argc, char* args[] )
{
  srand(time(NULL));
  App app;
  app.debug = 0;
  memset(&app, 0, sizeof(app));

  app.state = STATE_MENU;
  app.menu.selected = MENU_NEW_GAME;
  app.credits = CREDITS_TEAM;

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

  init_font();
  InitializePathfinder();
  renderInit(&app);
  SDL_WM_SetCaption("survivor - pseudogames", "survivor");
  SDL_ShowCursor(0);
  app.icon = IMG_Load("data/icon.png");
  SDL_WM_SetIcon(app.icon, NULL);

  soundInit();
  loadItems(&app);
  loadEnemies(&app);
  loadMap(&app);
  app.game.player1.body.onHitSound = Mix_LoadWAV("sounds/ouch.wav");
  app.game.player2.body.onHitSound = Mix_LoadWAV("sounds/ouch.wav");
  setWave(&app, 0); // calls moveInit / gameInit

  while(app.state != STATE_EXIT){
	Uint32 startTime = SDL_GetTicks();
	movePrepare(&app);
	if (app.state == STATE_PLAYING) {
	  renderStart(&app);
	}
	bindKeyboard(&app);                                   

	if (app.state == STATE_PLAYING){
	  playRandomMusic();
	  addPowerup(&app);
	  spawnEnemy(&app);
	  move_enemies(&app);
	  renderFinish(&app);
	  checkGameover(&app);
	} else if (app.state == STATE_HELP) {
	  renderHelp(&app);
	} else if (app.state == STATE_CREDITS) {
	  renderCredits(&app);
	} else {
	  renderMenu(&app);
	  playMusic("menu.mp3", -1);
	}
	handleDelay(startTime);
  }

  Mix_FreeChunk(app.game.player1.body.onHitSound);
  Mix_FreeChunk(app.game.player2.body.onHitSound);
  Mix_FreeChunk(app.game.itemtype[ITEM_ENEMY_MEDIC].sound);
  Mix_FreeChunk(app.game.itemtype[ITEM_ENEMY_SOLDIER].sound);
  Mix_FreeChunk(app.game.itemtype[ITEM_PLAYER_BULLET].sound);
  Mix_FreeChunk(app.game.itemtype[ITEM_PLAYER_FLAME].sound);
  Mix_FreeChunk(app.game.itemtype[ITEM_BUILD].sound);
  Mix_FreeChunk(app.game.itemtype[ITEM_ENEMY_FASTER].sound);
  Mix_FreeChunk(app.game.itemtype[ITEM_ENEMY_SUICIDAL].sound);

  SDL_FreeSurface(app.game.board.base_image);
  SDL_FreeSurface(app.game.board.base_hit);
  SDL_FreeSurface(app.game.itemtype[ITEM_ENEMY_FASTER].hit_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_ENEMY_SUICIDAL].hit_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_ENEMY_MEDIC].hit_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_ENEMY_SOLDIER].hit_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_PLAYER_BULLET].hit_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_PLAYER_BULLET].image);
  SDL_FreeSurface(app.game.itemtype[ITEM_PLAYER_BULLET].shot_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_PLAYER_FLAME].image);
  SDL_FreeSurface(app.game.itemtype[ITEM_PLAYER_FLAME].hit_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_PLAYER_FLAME].shot_image);
  SDL_FreeSurface(app.game.itemtype[ITEM_HEALTH_PACK].image);
  SDL_FreeSurface(app.game.itemtype[ITEM_BUILD].image);
  SDL_FreeSurface(app.game.itemtype[ITEM_BUILD].hit_image);
  SDL_FreeSurface(app.game.enemy_class[ENEMY_MEDIC].image);
  SDL_FreeSurface(app.game.enemy_class[ENEMY_SOLDIER].image);
  SDL_FreeSurface(app.game.board.image);
  SDL_FreeSurface(app.game.board.hit);
  SDL_FreeSurface(app.icon);
  renderTerminate(&app);
  terminate_font();
  sound_terminate();
  SDL_Quit();


  return 0;
}


