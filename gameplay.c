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
	renderPlayer(app, &app->game.indy);
	renderPlayer(app, &app->game.allan);
	renderHead(app);

	checkPlayerLife(&app->game.indy);
	checkPlayerLife(&app->game.allan);
}

int near(Body *body1, Body *body2){
	//float a = body1->angle * M_PI / 180;
	//float dx = cos(a) * HOLD_DISTANCE;
	//float dy = sin(a) * HOLD_DISTANCE;

	int x1 = body1->pos.x; // + dx;
	int y1 = body1->pos.y; // - dy;
	int x2 = body2->pos.x;
	int y2 = body2->pos.y;
    float dx = x1 - x2;
    float dy = y1 - y2;

	float d = sqrt(dx * dx + dy* dy);

	printf("Deltas, f: %f\n", d);

	if( d < PICK_DISTANCE ) return 1;
	return 0;
}

void playerChargeSpecialAttack(App *app, Player *player){
	Body *body = &player->body;
	Body *head_body = &app->game.head.body;

	if(!player->grabbing && near(body, head_body)){
		float a = body->angle * M_PI / 180;
		float dx = cos(a) * HOLD_DISTANCE;
		float dy = sin(a) * HOLD_DISTANCE;
		float tx = head_body->pos.x - dx;
		float ty = head_body->pos.y + dy;


		if(is_empty(&app->game, body, (int)tx,(int)body->pos.y))
			body->pos.x = tx;
		if(is_empty(&app->game, body, (int)body->pos.x,(int)ty))
			body->pos.y = ty;

		if(is_empty(&app->game, body, (int)tx,(int)ty)) {
			player->grabbing = 1;
			head_body->action = ACTION_ATTACK;
		}

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
