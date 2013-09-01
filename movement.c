#include "movement.h"

#define AI_FREQ 500
#define AI_PER_FRAME 20
#define ZOMBIE_MEMORY1_FLUSH 600;
#define ZOMBIE_MEMORY2_FLUSH 4000;
#define HIT_FREQ 200
#define TiHIT_FREQ 200
#define ATAN2(dx,dy) ((int)(720+atan2(-(dy),(dx))*180/M_PI)%360) // FIXME wrap angle properly
#define TILESIZE 32

inline int is_solid(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;

	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		return 1;
	if(body->pos.x/tileSize == x && body->pos.y/tileSize == y)
		return 0;
	return game->board.crowd[x][y];
}

inline int is_empty(Game *game, Body *body, int x, int y)
{
	return !is_solid(game, body,x,y);
}

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

void player_move(App *app, Player *player, int up, int right, int down, int left, int halt)
{
	Body *body = &player->body;

	if(body->action == ACTION_DEATH) return;

    int dx=right-left;
    int dy=down-up;
    if(dx||dy) {
		if(body->action == ACTION_MOVE){
			body->frame += 1;
			if((int)body->frame > body->sprite->frame_count) {
				body->frame = 0;
			}
		}

        float angle = ATAN2(dx,dy);
        body_move(&app->game, body, angle, !halt);
    }
}


/////

void movePrepare(App *app)
{
	int i, x,y;
	memcpy(walkability, app->game.board.wall, sizeof(app->game.board.wall)); // 1 = totaly unwalkable
	memcpy(app->game.board.crowd, app->game.board.wall, sizeof(app->game.board.wall));
	memcpy(app->game.board.hittable, app->game.board.air, sizeof(app->game.board.air));

	for(i=0; i < app->game.board.spawn_count; i++)
	{
		int x = app->game.board.spawn[i].x;
		int y = app->game.board.spawn[i].y;
		walkability[x][y] = 0; // totally walkable
	}

	{
		int x = app->game.indy.body.pos.x/tileSize;
		int y = app->game.indy.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 2;
	}

	{
		int x = app->game.allan.body.pos.x/tileSize;
		int y = app->game.allan.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 3;
	}

	for(i=0; i < ENEMY_COUNT; i++)
	{
		if(app->game.board.enemies[i].alive)
		{
			int x = app->game.board.enemies[i].body.pos.x/tileSize;
			int y = app->game.board.enemies[i].body.pos.y/tileSize;
			app->game.board.crowd[x][y] = 4+i;
			app->game.board.hittable[x][y] = 4+i;
			walkability[x][y] = 2+mapWidth/4;
		}
	}

	int t = SDL_GetTicks();
	int flush1 = t > app->game.board.zombie_memory1 + ZOMBIE_MEMORY1_FLUSH;
	int flush2 = t > app->game.board.zombie_memory2 + ZOMBIE_MEMORY2_FLUSH;
	int death1a[mapWidth][mapHeight];
	int death2a[mapWidth][mapHeight];
	if(flush1) {
		memset(death1a, 0, sizeof(death1a));
		app->game.board.zombie_memory1 = t;
	}
	if(flush2) {
		memset(death2a, 0, sizeof(death2a));
		app->game.board.zombie_memory2 = t;
	}
	int death2_max = 0;
	for (x=0; x < mapWidth;x++) {
		for (y=0; y < mapHeight;y++) {
			if(death2_max < app->game.board.death2[x][y])
				death2_max = app->game.board.death2[x][y];
		}
	}
	for (x=0; x < mapWidth;x++) {
		for (y=0; y < mapHeight;y++) {
			if(flush1||flush2) {
				int xx, yy;
				int xx0 = x-1 > 0 ? x-1 : 0;
				int xx1 = x+1 < mapWidth ? x+1 : mapWidth-1;
				int yy0 = y-1 > 0 ? y-1 : 0;
				int yy1 = y+1 < mapHeight ? y+1 : mapHeight-1;

				if(flush1) {
					for (xx=xx0; xx<=xx1; xx++) {
						for (yy=yy0; yy<=yy1; yy++) {
							death1a[x][y] 
								+= app->game.board.death1[xx][yy] * (x==xx&&y==yy ? 600 : 100);
						}
					}
					death1a[x][y] /= 1410;
				}

				if(flush2) {
					for (xx=xx0; xx<=xx1; xx++) {
						for (yy=yy0; yy<=yy1; yy++) {
							death2a[x][y] 
								+= app->game.board.death2[xx][yy] * (x==xx&&y==yy ? 200 : 100);
						}
					}
					death2a[x][y] /= 1000;
				}

			}
			int d1 = app->game.board.death1[x][y];
			float d2 = app->game.board.death2[x][y]/(float)death2_max;
			int cost = mapWidth*(d1/100. + 2*d2);
			if(walkability[x][y] != 1 && cost > 0) {
				walkability[x][y] = 1+cost;
			}
		}
	}
	if(flush1) {
		memcpy(app->game.board.death1, death1a, sizeof(death1a));
	}
	if(flush2) {
		memcpy(app->game.board.death2, death2a, sizeof(death2a));
	}
}


void moveInit(App *app)
{
	int x,y;
	SDL_Surface *hit = app->game.board.hit;
	memset(app->game.board.wall, 0, sizeof(app->game.board.wall));
	memset(app->game.board.air, 0, sizeof(app->game.board.air));
	memset(app->game.board.spawn_map, 0, sizeof(app->game.board.spawn_map));
	app->game.board.spawn_count = 0;
	for (x=0; x < mapWidth;x++) {
		for (y=0; y < mapHeight;y++) {
			Uint32 *p = (Uint32*)( ((Uint8*)hit->pixels) + (x*hit->format->BytesPerPixel+y*hit->pitch) );
			Uint8 r,g,b;
			SDL_GetRGB(*p, hit->format, &r, &g, &b);
			int threshold = 0x40;
			int p1 = g<r-threshold && g<b-threshold;
			int p2 = r<g-threshold && r<b-threshold;
			int head = b<r-threshold && b<g-threshold;
			int walk = b+g>r+threshold || p1 || p2 || head;
			int fly = r+g+b>threshold || p1 || p2 || head;
			int safe = g>r+b+threshold;
			app->game.board.wall[x][y] = !walk;
			app->game.board.air[x][y] = !fly;
			if(safe) {
				app->game.board.spawn_map[x][y]=1;
				app->game.board.spawn[app->game.board.spawn_count].x = x;
				app->game.board.spawn[app->game.board.spawn_count].y = y;
				app->game.board.spawn_count++;
			}
			if(p1) {
				app->game.indy.body.pos.x = x * tileSize + tileSize/2;
				app->game.indy.body.pos.y = y * tileSize + tileSize/2;
			}
			if(p2) {
				app->game.allan.body.pos.x = x * tileSize + tileSize/2;
				app->game.allan.body.pos.y = y * tileSize + tileSize/2;
			}
			if(head) {
				app->game.head.body.pos.x = x * tileSize + tileSize/2;
				app->game.head.body.pos.y = y * tileSize + tileSize/2;
			}
		}
	}
	movePrepare(app);
}



