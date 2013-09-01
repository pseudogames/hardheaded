#include "movement.h"

#define AI_FREQ 500
#define AI_PER_FRAME 20
#define ZOMBIE_MEMORY1_FLUSH 600;
#define ZOMBIE_MEMORY2_FLUSH 4000;
#define HIT_FREQ 200
#define TiHIT_FREQ 200
#define ATAN2(dx,dy) ((int)(720+atan2(-(dy),(dx))*180/M_PI)%360) // FIXME wrap angle properly
#define TILESIZE 32

void angle_rotate(float *a0_base, float a1, float f)
{
	float a0 = *a0_base;
	if(fabs(a1 - a0) > 180) {
		if(a0 < a1)
			a0 += 360;
		else
			a1 += 360;
	}
	// FIXME wrap angle properly
	a0 = (720+a0)*(1-f) + f*(720+a1);
	while(a0>360) a0-=360;
	*a0_base = a0;
}

void body_move(Game *game, Body *body, float angle, float vel)
{
	float v = body->max_vel * vel;

	angle_rotate(&body->angle, angle, body->ang_vel);
#define ANGLE_STEP 15
	int a0 = round(body->angle / ANGLE_STEP) * ANGLE_STEP;
//	fprintf(stderr, "%f %f\n", body->angle, a0);
	float a = a0 * M_PI / 180.;
	float dx = cos(a) * v;
	float dy = sin(a) * v;
	int x0 = body->pos.x;
	int y0 = body->pos.y;
	int x1 = x0 + dx;
	int y1 = y0 - dy;


	int mx = x0/TILESIZE- x1/TILESIZE;
	int my = y0/TILESIZE- y1/TILESIZE;
	if(!mx) body->pos.x = x1;
	if(!my) body->pos.y = y1;
	if(mx || my) {
		if(is_empty(game, body,x1,y1)) {
			body->pos.x = x1;
			body->pos.y = y1;
			//printf("xy: ");
		} else {
			if(mx && is_empty(game, body,x1,y0)) {
				body->pos.x = x1;
			} else if(my && is_empty(game, body,x0,y1)) {
				body->pos.y = y1;
			}
		}
	}
}

int is_empty(Game *game, Body *body, int x, int y)
{
	return 1;
}

void player_move(App *app, Body *body, int up, int right, int down, int left, int halt)
{

	if(body->action == ACTION_DEATH) return;

	if((int)body->frame > body->sprite->frame_count) {
		body->frame = 0;
	}

    int dx=right-left;
    int dy=down-up;
    if(dx||dy) {
		if(body->action == ACTION_MOVE){
			body->frame += 1;
		}
        float angle = ATAN2(dx,dy);
        body_move(&app->game, body, angle, !halt);
    }
}
