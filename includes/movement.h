#ifndef _MOVEMENT_H
#define _MOVEMENT_H
#include <math.h>
#include "hardheaded.h"
#include "gameplay.h"

void player_move(App *app, Player *player, int up, int right, int down, int left, int halt);
#endif
