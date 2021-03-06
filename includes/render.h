#ifndef _RENDER_H
#define _RENDER_H
#include "hardheaded.h"
#include "font.h"
#include "movement.h"

extern SDL_Color red;
extern SDL_Color trueRed;
extern SDL_Color white;
extern SDL_Color green;
extern SDL_Color yellow;
extern SDL_Color black;

void renderInit(App *app);
void renderBody(App *app, Body *body);
void renderPlayer(App *app, Player *player);
void renderPlayerLife(App *app, SDL_Surface *screen, Player *player, int playerOffset);
void renderPlayerSpecialBar(App *app, SDL_Surface *screen, Player *player, int playerOffset);
void renderHead(App *app);
void renderWinner(App *app);
void renderGameOver(App *app);
void renderSpawnCountdown(App *app);
#endif
