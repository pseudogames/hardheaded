#ifndef _GAMEPLAY_H
#define _GAMEPLAY_H
#include "hardheaded.h"
#include "font.h"
#include "render.h"
#include "movement.h"

#define HOLD_DISTANCE 16
#define PICK_DISTANCE 32

void renderGameplay(App *app);
void playerAttack(App *app, Player *player);
void playerChargeSpecialAttack(App *app, Player *player);
void playerDie(App *app, Player *player);
int hit(App *app, Body *source, Body *target);
int shoot(App *app, Body *body, int spread, int angle, float range);

#endif
