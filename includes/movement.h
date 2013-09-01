#ifndef _MOVEMENT_H
#define _MOVEMENT_H
#include <math.h>
#include "hardheaded.h"
#include "gameplay.h"

void movePlayer(App *app, Player *player, int up, int right, int down, int left, int halt);
void moveEnemies(App *app);
inline int is_solid(Game *game, Body *body, int x, int y);
inline int is_air(Game *game, Body *body, int x, int y);
inline int is_hit(Game *game, Body *body, int x, int y);

#endif
