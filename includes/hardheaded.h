#ifndef _HARDHEADED_H
#define _HARDHEADED_H
#include <SDL.h>
#include <SDL_mixer.h>
#include "sprite.h"

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
  Player indy;
  Player allan;
  Player zombie;
  Player head;
  Hearts hearts;
  SDL_Surface *special_bar;
} Board;

typedef struct {
  Board board;
} Game;

typedef struct {
  SDL_Surface *screen;
  AppState state;
  Game game;
  Menu menu;
  SDL_Surface *logo;
} App;

#endif
