#ifndef _HARDHEADED_H
#define _HARDHEADED_H
#include <SDL.h>
#include <SDL_mixer.h>
#include "sprite.h"

#define DAMAGE 0.75
#define ZOMBIE_HEALTH DAMAGE
#define HEAD_HEALTH (20*DAMAGE)
#define PLAYER_HEALTH (12*DAMAGE)
#define ZOMBIE_SCORE (DAMAGE/4)
#define WAVE_COUNT 32
#define ENEMY_COUNT 1000
#include "aStarLibrary.h" // must be after the defines above

typedef enum {
	DEBUG_NONE,
	DEBUG_WALL,
	DEBUG_AIR,
	DEBUG_AI,
	DEBUG_ENEMY,
	DEBUG_MOVE,
	DEBUG_SHOT,
	DEBUG_DEATH1,
	DEBUG_DEATH2,
	DEBUG_COUNT
} Debug;

typedef enum {
  STATE_MENU,
  STATE_PAUSED,
  STATE_PLAYING,
  STATE_GAMEOVER,
  STATE_HELP,
  STATE_CREDITS,
  STATE_EXIT,
} AppState;

typedef enum {
  CREDITS_TEAM = 0,
  CREDITS_SOUND
} Credits;

typedef enum {
  MENU_RESUME = 0,
  MENU_NEW_GAME,
  MENU_CREDITS,
  MENU_EXIT,
  MENU_COUNT
} MenuItem;

typedef struct {
  MenuItem selected;
  SDL_Surface *indiana;
} Menu;

typedef struct { float x,y; } vec;

typedef struct {
  vec pos;
  Action action;
  float angle; // degree
  float frame;
  float ang_vel;
  float max_vel;
  float vel;
  Sprite *sprite;
  float life;
  float score;
} Body;

typedef struct{
  Sprite sprite;
  Sprite power;
  Body body;
  Body power_body;
  int special_attack;
  char* name;
  int grabbing;
  int last_ai;
  point door;
} Player;

typedef struct{
	SDL_Surface *full;
	SDL_Surface *empty;
	SDL_Surface *onequarter;
	SDL_Surface *twoquarter;
	SDL_Surface *threequarter;
}Hearts;

typedef struct{
  Body body;
  int pathfinder;
  int pathfinder_other;
  int pathfinder_another;
  Player *target;
  int alive;
  int variation;
  int last_ai;
} Enemy;

typedef struct {
	int time;
	int enemy_spawn_interval;
	int enemy_count_per_spawn;
	int enemy_count_on_screen;
	int enemy_count;
	int enemy_variation;
} Wave;

typedef struct{
  SDL_Surface *image;
  SDL_Surface *hit;

  Enemy enemies[ENEMY_COUNT];
  int latest_enemy_updated;

  int wave_start;
  Uint32 spawnTime;
  int kill_count;
  int total_enemies;
  int on_screen_enemies;

  int wall[mapWidth][mapHeight];
  int air[mapWidth][mapHeight];
  int crowd[mapWidth][mapHeight];
  int hittable[mapWidth][mapHeight];

  int spawn_map[mapWidth][mapHeight];
  point spawn[mapWidth*mapHeight];
  int spawn_count;

  int death1[mapWidth][mapHeight];
  int death2[mapWidth][mapHeight];
  int zombie_memory1;
  int zombie_memory2;
} Board;

typedef struct {
  Wave wave[WAVE_COUNT];
  Board board;
  Player indy;
  Player allan;
  Player head;
  Uint32 start;
  int total_kill_count;
  int next_wave;
  int wave_count;
  int wave_index;
  Player *winner;
} Game;

typedef struct {
  SDL_Surface *screen;
  Sprite zombie;
  Hearts hearts;
  AppState state;
  Game game;
  int credits;
  Menu menu;
  SDL_Surface *logo;
  SDL_Surface *special_bar;
  Debug debug;
  int enemy_frame_count;
  int enemy_variation_count;
} App;

#endif
