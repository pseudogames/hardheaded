#ifndef _HARDHEADED_H
#define _HARDHEADED_H
#include <SDL.h>
#include <SDL_mixer.h>
#include "sprite.h"

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
  MENU_EXIT,
  MENU_COUNT
} MenuItem;

typedef struct {
  MenuItem selected;
  SDL_Surface *indiana;
} Menu;

typedef struct {
  SDL_Rect pos;
  Action action;
  float angle; // degree
  float frame;
  float ang_vel;
  float max_vel;
  Sprite *sprite;
} Body;

typedef struct{
  Sprite sprite;
  Body body;
  int special_attack;
  float life;
  char* name;
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
  Body *target;
  int alive;
} Enemy;

typedef struct{
  SDL_Surface *image;
  SDL_Surface *hit;

  Enemy enemies[ENEMY_COUNT];
  int latest_enemy_updated;

  int wave_count;
  int wave_index;
  int wave_start;

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
  Board board;
  Player indy;
  Player allan;
  Player head;
  Uint32 start;
  Uint32 spawnTime;
  int kill_count;
  int total_enemies;
  int on_screen_enemies;
} Game;

typedef struct {
  SDL_Surface *screen;
  Sprite zombie;
  Hearts hearts;
  AppState state;
  Game game;
  Menu menu;
  SDL_Surface *logo;
  SDL_Surface *special_bar;
  Debug debug;
} App;

#endif
