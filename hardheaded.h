#ifndef _HARDHEADED_H
#define _HARDHEADED_H
#include <SDL.h>
#include <SDL_mixer.h>

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

typedef struct { int x,y; } point;

typedef struct {
    point origin;
    point frame_size;
    int frame_count;
    SDL_Surface *source;
    point rotated_frame_size;
    SDL_Surface *rotated;
} Sprite;

typedef struct{
  Sprite indy;
} Board;

typedef struct {
  SDL_Surface *screen;
  AppState state;
  Board board;
} App;

typedef struct {

} Menu;

#endif
