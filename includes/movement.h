#ifndef _MOVEMENT_H
#define _MOVEMENT_H
#include <math.h>
#include "hardheaded.h"

void player_move(App *app, Body *body, int up, int right, int down, int left, int halt);
#endif
