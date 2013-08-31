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

typedef struct {
  SDL_Rect pos;
  Action action;
  float angle; // degree
  float frame;
  Sprite *sprite;
} Body;

typedef struct{
  Sprite sprite;
  Body body;
} Player;

typedef struct{
  Player indy;
  Player zombie;
  Player head;
} Board;

typedef struct {
  Board board;
} Game;

typedef struct {
  SDL_Surface *screen;
  AppState state;
  Game game;
} App;

typedef struct {

} Menu;

#endif
