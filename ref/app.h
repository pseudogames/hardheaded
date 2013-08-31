#ifndef _APP_H
#define _APP_H
#include <SDL.h>
#include <SDL_mixer.h>

#define ENEMY_COUNT 1000
#define POWERUP_COUNT 32
#define PLAYER_COUNT 2
#define SPRITE_COUNT (PLAYER_COUNT+ENEMY_COUNT+POWERUP_COUNT)

#define BUILD_LIMIT 200
#define WAVE_COUNT 256

#include "aStarLibrary.h" // must be after the defines above

enum {
	ITEM_HEALTH_PACK,
	ITEM_PLAYER_BULLET,
	ITEM_PLAYER_FLAME,
	ITEM_PLAYER_COUNT,
	ITEM_ENEMY_MEDIC,
	ITEM_ENEMY_SOLDIER,
	ITEM_ENEMY_FASTER,
	ITEM_ENEMY_SUICIDAL,
	ITEM_BUILD,
	ITEM_COUNT
};

enum {
  ENEMY_MEDIC,
  ENEMY_SOLDIER,
  ENEMY_FASTER,
  ENEMY_SUICIDAL,
  ENEMY_TYPE_COUNT
};

typedef enum {
  MENU_RESUME = 0,
  MENU_NEW_GAME,
  MENU_HELP,
  MENU_CREDITS,
  MENU_QUIT,
  MENU_COUNT
} MenuItem;

typedef struct {
  MenuItem selected;
  SDL_Surface *soldier;
  SDL_Surface *zombie;
  SDL_Surface *engineer;
  SDL_Surface *bigZombie;
  SDL_Surface *keyBinds;
} Menu;

typedef enum {
  BODY_DEAD = 0,
  BODY_ALIVE
} BodyStatus;

typedef struct {
  int spread;
  int freq;
  SDL_Surface *shot_image;
  SDL_Surface *hit_image;
  SDL_Surface *image;
  Mix_Chunk *sound;
  float damage;
  float range;
  int ammo_total;
  int score;
  int chance;
  int should_explode;
} ItemType;

typedef struct {
  ItemType *type;
  int ammo_used;
  int should_show;
  int x;
  int y;
} Item;

typedef struct { int x,y; } point;

typedef struct {
    point origin;
    point frame_size;
    int frame_count;
    SDL_Surface *source;
    point rotated_frame_size;
    SDL_Surface *rotated;
} Sprite;

typedef enum { 
    ACTION_MOVE=0, 
    ACTION_ATTACK, 
    ACTION_DEATH, 
    ACTION_COUNT
} Action;

typedef struct {
  SDL_Rect pos;
  Action action;
  int frame;
  float life;
  int ammo;
  Item item;
  BodyStatus status;
  float max_vel;
  float ang_vel;
  float angle; // degree
  int shoot_key; // OH GOD WHY!
  Mix_Chunk *onHitSound;
  int last_ai; // enemy=last pathfind; player=last time was reached
  int exploded;
  Sprite *sprite;
} Body;

typedef struct{
  Body body;
} Player;

typedef struct{
  Sprite sprite;
  ItemType *type;
  float max_life;
  float vel;
} EnemyClass;

typedef struct{
  Body body;
  int pathfinder;
  int pathfinder_other;
  Body *target;
} Enemy;

typedef struct {
	int x,y;
	int open;
} Spawn;

typedef struct {
	SDL_Rect src;
	SDL_Rect dst;
	SDL_Surface *image;
} Blit;

typedef struct {
	int x, y;
	int w, h;
	int enemy_chance[ENEMY_TYPE_COUNT];
	int required_kills;
	int enemy_spawn_interval;
	int enemy_count_per_spawn;
	int enemy_count_on_screen;
	int enemy_count;
} Wave;

typedef struct{
  Sprite indy;
  Sprite alan;
  Sprite zombie;
  Sprite head;
  SDL_Surface *base_image;
  SDL_Surface *base_hit;
  SDL_Surface *image;
  SDL_Surface *hit;
  Wave wave[WAVE_COUNT];
  int wave_count;
  int wave_index;
  int wave_start;
  int wall[mapWidth][mapHeight];
  int air[mapWidth][mapHeight];
  int crowd[mapWidth][mapHeight];
  int hittable[mapWidth][mapHeight];
  int powerup[mapWidth][mapHeight];
  int safearea[mapWidth][mapHeight];
  int death1[mapWidth][mapHeight];
  int death2[mapWidth][mapHeight];
  int spawn_map[mapWidth][mapHeight];
  Spawn spawn[mapWidth*mapHeight];
  int spawn_count;
  Blit blit[SPRITE_COUNT];
  int blit_count;
  Item powerups[POWERUP_COUNT];
  int zombie_memory1;
  int zombie_memory2;
} Board;

typedef struct {
  SDL_Surface *image;
} HealthPack;

typedef struct {
  Player player1;
  Player player2;
  Enemy enemies[ENEMY_COUNT];
  Board board;
  Uint32 start;
  Uint32 spawnTime;
  Uint32 spawnPowerupTime;
  int latest_enemy_updated;
  int kill_count;
  int total_kill_count;
  int won;
  int total_enemies;
  int on_screen_enemies;
  HealthPack health_pack;
  ItemType itemtype[ITEM_COUNT];
  EnemyClass enemy_class[ENEMY_TYPE_COUNT];
  int hint_pivot;
  int hint_grab;
} Game;

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
	DEBUG_NONE,
	DEBUG_WALL,
	DEBUG_AIR,
	DEBUG_AI,
	DEBUG_ENEMY,
	DEBUG_MOVE,
	DEBUG_SHOT,
	DEBUG_ITEM,
	DEBUG_SAFE,
	DEBUG_DEATH1,
	DEBUG_DEATH2,
	DEBUG_COUNT
} Debug;

typedef struct {
  SDL_Surface *screen;
  Game game;
  Credits credits;
  Debug debug;
  AppState state;
  /**
   * just to handle if is to show or not the resume page
   * TODO: should be refactored to not use this attr
   */
  AppState stateBeforeCredits;
  SDL_Surface *logo;
  SDL_Surface *icon;
  Menu menu;
} App;

int hit(App *app, Body *source, Body *target);

#endif
