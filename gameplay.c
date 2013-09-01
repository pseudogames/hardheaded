#include "gameplay.h"
#include <math.h>

void renderLifeBars(App *app){
	SDL_Surface *screen = app->screen;
	Board *board = &app->game.board;

	renderPlayerSpecialBar(app, screen, &app->game.indy, 0);
	renderPlayerSpecialBar(app, screen, &app->game.allan, 650);

	renderPlayerLife(app, screen, &app->game.indy, 0);
	renderPlayerLife(app, screen, &app->game.allan, 650);
}

void checkPlayerLife(Player *player){
	if(player->life <= 10 && player->body.action == ACTION_DEATH){
		player->life += 0.05;
		if(player->life > 10) player->life = 10;
	}
}

void renderGameplay(App *app){
	SDL_BlitSurface(app->game.board.image, NULL, app->screen, NULL);

	renderDebug(app);

	renderLifeBars(app);
	renderBody(app, &app->game.indy.body, &app->game.indy);
	renderBody(app, &app->game.allan.body, &app->game.allan);
	renderHead(app);

	checkPlayerLife(&app->game.indy);
	checkPlayerLife(&app->game.allan);
}

int near(Body *body1, Body *body2){
	int tileset = 32;
	int x1 = body1->pos.x/tileset;
	int y1 = body1->pos.y/tileset;
	int x2 = body2->pos.x/tileset;
	int y2 = body2->pos.y/tileset;
    int dx = fabs(x1 - x2);
    int dy = fabs(y1 - y2);

//	printf("Deltas, x: %i, y: %i", dx, dy);

	if( dx < 3 && dy < 3){
		return 1;
	}
	return 0;
}

void playerChargeSpecialAttack(App *app, Player *player){
	Body *body = &player->body;
	Body *head_body = &app->game.head.body;

	if(near(body, head_body)){
		player->grabbing = 1;
		head_body->action = ACTION_ATTACK;
	}

	if(!player->grabbing && player->special_attack < 100) {
		player->special_attack += 1;
	}

}

void playerAttack(App *app, Player *player){
	Body *body = &player->body;
	Body *head_body = &app->game.head.body;

	if(body->action == ACTION_DEATH) return;
	if(player->grabbing){
		head_body->action = ACTION_MOVE;
		player->grabbing = 0
		;return;
	}

	if(player->special_attack < 100) {
		body->action = ACTION_ATTACK;
		body->frame = 0;
	}

	player->special_attack = 0;
}

void playerDie(App *app, Player *player){
	player->life = 0;
	Body *body = &player->body;
	body->action = ACTION_DEATH;
	body->frame = 0;
}
