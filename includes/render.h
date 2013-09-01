#ifndef _RENDER_H
#define _RENDER_H
#include "hardheaded.h"
#include "font.h"

extern SDL_Color red;
extern SDL_Color trueRed;
extern SDL_Color white;
extern SDL_Color green;
extern SDL_Color yellow;

void renderInit(App *app);
void renderStart(App *app);
void renderBody(App *app, Body *body, Player *player);
void renderPlayerLife(App *app, SDL_Surface *screen, Player *player, int playerOffset);
void renderPlayerSpecialBar(App *app, SDL_Surface *screen, Player *player, int playerOffset);
void renderFinish(App *app);
#endif
