#include <math.h>
#include "app.h"

void player_move(App *app, Body *body, int up, int right, int down, int left, int halt);
void moveInit(App *app);
void movePrepare(App *app);
void move_enemies(App *app);
int player_spawn_pos(Game *game, Uint16 *x, Uint16 *y);
int enemy_spawn_pos(Game *game, int *x, int *y);
int powerup_spawn_pos(Game *game, int *x, int *y);
