#ifndef _SPRITE_H
#define _SPRITE_H

#include <SDL.h>

typedef enum { 
    ACTION_MOVE=0, 
    ACTION_ATTACK, 
    ACTION_DEATH, 
    ACTION_COUNT
} Action;

typedef struct { int x,y; } point;

typedef struct {
    point origin;
    point frame_size;
    int frame_count;
    SDL_Surface *source;
    point rotated_frame_size;
    SDL_Surface *rotated;
} Sprite;

void sprite_init(Sprite *sprite, int ox, int oy, int fx, int fy, int c, const void *mem, int len);
void sprite_rotated_rect(Sprite *sprite, Action action, int frame, int angle, SDL_Rect *rect);

#endif
