#include "movement.h"
#include "aStarLibrary.h"

#define AI_FREQ 500
#define AI_PER_FRAME 20
#define ZOMBIE_MEMORY1_FLUSH 600;
#define ZOMBIE_MEMORY2_FLUSH 4000;
#define HIT_FREQ 200
#define ATAN2(dx,dy) ((int)(720+atan2(-(dy),(dx))*180/M_PI)%360) // FIXME wrap angle properly

void movePrepare(App *app)
{
	int i, x,y;
	memcpy(walkability, app->game.board.wall, sizeof(app->game.board.wall)); // 1 = totaly unwalkable
	memcpy(app->game.board.crowd, app->game.board.wall, sizeof(app->game.board.wall));
	memcpy(app->game.board.hittable, app->game.board.air, sizeof(app->game.board.air));

	for(i=0; i < app->game.board.spawn_count; i++)
	{
		if(app->game.board.spawn[i].open)
		{
			int x = app->game.board.spawn[i].x;
			int y = app->game.board.spawn[i].y;
			walkability[x][y] = 0; // totally walkable
		}
	}

	if( app->game.player1.body.status == BODY_ALIVE) {
		int x = app->game.player1.body.pos.x/tileSize;
		int y = app->game.player1.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 2;
	}

	if( app->game.player2.body.status == BODY_ALIVE) {
		int x = app->game.player2.body.pos.x/tileSize;
		int y = app->game.player2.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 3;
	}

	for(i=0; i < ENEMY_COUNT; i++)
	{
		if(app->game.enemies[i].body.status == BODY_ALIVE)
		{
			int x = app->game.enemies[i].body.pos.x/tileSize;
			int y = app->game.enemies[i].body.pos.y/tileSize;
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
	int hit_built = rand()%2;
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
			float b = app->game.board.built[x][y]/(float)BUILD_LIMIT;
			int bb = !!b;
			if(app->game.board.crowd[x][y] < bb)
				app->game.board.crowd[x][y] = bb;
			if(hit_built && app->game.board.hittable[x][y] < bb)
				app->game.board.hittable[x][y] = bb;
			int cost = mapWidth*(2*b + d1/100. + 2*d2);
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
	memset(app->game.board.safearea, 0, sizeof(app->game.board.safearea));
	memset(app->game.board.spawn_map, 0, sizeof(app->game.board.spawn_map));
	app->game.board.spawn_count = 0;
	for (x=0; x < mapWidth;x++) {
		for (y=0; y < mapHeight;y++) {
			Uint32 *p = (Uint32*)( ((Uint8*)hit->pixels) + (x*hit->format->BytesPerPixel+y*hit->pitch) );
			Uint8 r,g,b;
			SDL_GetRGB(*p, hit->format, &r, &g, &b);
			int threshold = 0x40;
			int walk = b+g>r+threshold;
			int fly = r+g+b>threshold;
			int safe = g>r+b+threshold;
			app->game.board.wall[x][y] = !walk;
			app->game.board.air[x][y] = !fly;
			app->game.board.safearea[x][y] = safe;
			if(walk && (x==0 || y==0 || x==mapWidth-1 || y==mapHeight-1
				|| x == (app->game.board.wave[app->game.board.wave_index].w)-1
				|| y == (app->game.board.wave[app->game.board.wave_index].h)-1
			)) {
				app->game.board.spawn_map[x][y]=1;
				app->game.board.spawn[app->game.board.spawn_count].x = x;
				app->game.board.spawn[app->game.board.spawn_count].y = y;
				app->game.board.spawn[app->game.board.spawn_count].open = 1;
				app->game.board.spawn_count++;
			}
		}
	}
	movePrepare(app);
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

inline int is_solid(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;

	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		return 1;
	if(body->pos.x/tileSize == x && body->pos.y/tileSize == y)
		return 0;
	if(game->board.built[x][y]>0) {
		game->board.built[x][y]--;
		if(game->board.built[x][y]<0) 
			game->board.built[x][y]=0;
	}
	return game->board.crowd[x][y];
}
inline int is_empty(Game *game, Body *body, int x, int y)
{
	return !is_solid(game, body,x,y);
}

void body_move(Game *game, Body *body, float angle, float vel)
{
	if(body->status== BODY_DEAD){
		return;
	}

	float v = body->max_vel * vel;

	angle_rotate(&body->angle, angle, body->ang_vel);
	float a = body->angle * M_PI / 180.;
	float dx = cos(a) * v;
	float dy = sin(a) * v;
	int x0 = body->pos.x;
	int y0 = body->pos.y;
	int x1 = x0 + dx;
	int y1 = y0 - dy;


	int mx = x0/tileSize - x1/tileSize;
	int my = y0/tileSize - y1/tileSize;
	if(!mx) body->pos.x = x1;
	if(!my) body->pos.y = y1;
	if(mx || my) {
		if(is_empty(game, body,x1,y1)) {
			body->pos.x = x1;
			body->pos.y = y1;
			//printf("xy: ");
		} else {
			if(mx && is_empty(game, body,x1,y0)) {
				//printf("x %d %d: ",x0,x1);
				body->pos.x = x1;
			} else if(my && is_empty(game, body,x0,y1)) {
				//printf("y %d %d: ",y0,y1);
				body->pos.y = y1;
			}
		}
	}

	//printf("body: angle: %f a: %f 2pi:%f y:%f\n", body->angle, a, 2*M_PI, sin(a) * v);
	//body->frame = (body->frame+(rand()%2)) % body->sprite->frame_count;
}

void move_enemies(App *app)
{
  int i;
  int t = SDL_GetTicks();
  for(i=0; i < ENEMY_COUNT; i++)
  {
    if(app->game.enemies[i].body.status == BODY_ALIVE
	&& app->game.enemies[i].body.life <= 0) {
		app->game.enemies[i].body.status = BODY_DEAD;
	}
  }

  //printf("-------------\n");
  int n=AI_PER_FRAME;
  for(i=0; i < ENEMY_COUNT && n > 0; i++) 
  {
    int id = app->game.latest_enemy_updated = ( app->game.latest_enemy_updated + 1 ) % ENEMY_COUNT;
    int crazy = id*2;
    if(app->game.enemies[id].body.status == BODY_ALIVE && t > app->game.enemies[id].body.last_ai+AI_FREQ)
    {
        Body *enemy_body = &app->game.enemies[id].body;
		n--;
		//printf("path %d %d %d\n", i, n, id);
		app->game.enemies[id].body.last_ai = t;
		pathLength[crazy] = 9999;
		pathLength[crazy+1] = 9999;

		if(app->game.player1.body.status == BODY_ALIVE) {
			//printf("find %d=%d\n", id, crazy);
			pathStatus[crazy] = FindPath(crazy,
				enemy_body->pos.x,
				enemy_body->pos.y,
				app->game.player1.body.pos.x,
				app->game.player1.body.pos.y);
		}

		if(app->game.player2.body.status == BODY_ALIVE) {
			//printf("find %d=%d\n", id, crazy+1);
			pathStatus[crazy+1] = FindPath(crazy+1,
				enemy_body->pos.x,
				enemy_body->pos.y,
				app->game.player2.body.pos.x,
				app->game.player2.body.pos.y);
		}


        if(pathLength[crazy] < pathLength[crazy+1]){
          app->game.enemies[id].pathfinder = crazy;
          app->game.enemies[id].pathfinder_other = crazy+1;
          app->game.enemies[id].target = &app->game.player1.body;
        }else{
          app->game.enemies[id].pathfinder = crazy+1;
          app->game.enemies[id].pathfinder_other = crazy;
          app->game.enemies[id].target = &app->game.player2.body;
        }


#if 0
		{
			extern int* pathBank [numberPeople+1];
			int k;
			int j = app->game.enemies[id].pathfinder;
			printf("%d (%d)", j, pathLength[j]);
			for(k=0; k<pathLength[j]; k+=2) {
				printf("%d,%d; ", pathBank[j][k], pathBank[j][k+1]);
			}
			printf("\n");
		}
#endif 

    }
  }

  for(i = 0; i < ENEMY_COUNT; i++)
  {
    if(app->game.enemies[i].body.status == BODY_ALIVE)
    {
        Body *enemy_body = &app->game.enemies[i].body;
		int crazy = app->game.enemies[i].pathfinder;
        if(pathStatus[crazy] == found)
        {
			//printf("read %d=%d\n", i, crazy);
          int reach = ReadPath(crazy, enemy_body->pos.x, enemy_body->pos.y, tileSize*1.5);
          int dx = xPath[crazy] - enemy_body->pos.x;
          int dy = yPath[crazy] - enemy_body->pos.y;
          //float angle = ATAN2(dx,dy);
          float angle = (int)(720 + ATAN2(dx,dy) + sin((t/5000.+crazy/(float)AI_PER_FRAME)*M_PI)*30 ) % 360;
          body_move(&app->game, enemy_body, angle, .25+.75*rand()/(float)RAND_MAX);
		  //printf("enemy %d (%d, %d) angle %f -> %f\n", crazy, enemy_body->pos.x, enemy_body->pos.y, angle, enemy_body->angle);

			  
		  if(reach && (enemy_body->item.type->should_explode || 
					  t > app->game.enemies[i].target->last_ai+HIT_FREQ)){
			  app->game.enemies[i].target->last_ai = t;
			  //printf("reach %d=%d %d,%d\n", i, crazy, dx, dy);
			  pathStatus[app->game.enemies[i].pathfinder_other] = notStarted;
			  hit(app, enemy_body, app->game.enemies[i].target);

		  }
		}
	}
  }
}



void player_move(App *app, Body *body, int up, int right, int down, int left, int halt)
{
    aim(app, body);
    int dx=right-left;
    int dy=down-up;
    if(dx||dy) {
        float angle = ATAN2(dx,dy);
        body_move(&app->game, body, angle, !halt);
    }
}


int enemy_spawn_pos(Game *game, int *x, int *y) {
	if(!game->board.spawn_count) return 0;
	int i = rand() % game->board.spawn_count;
	// printf("%d %d\n", i, rand() % game->board.spawn_count);

	*x = game->board.spawn[i].x*tileSize+tileSize*3/4;
	*y = game->board.spawn[i].y*tileSize+tileSize*3/4;
	return 1;
}

int player_spawn_pos(Game *game, Uint16 *x, Uint16 *y)
{
	int i=0;
	//printf("--------\n");
	while(1) {
		int x1 = rand() % mapWidth;
		int y1 = rand() % mapHeight;
		//printf("try %d %d %d\n",i++,x1,y1);
		if(game->board.safearea[x1][y1] && !game->board.crowd[x1][y1]) {
			*x = x1 * tileSize + tileSize/2;
			*y = y1 * tileSize + tileSize/2;
			return 1;
		}
	}
	return 0;
}

int powerup_spawn_pos(Game *game, int *x, int *y) {
	int i;
	for(i=0; i< 50; i++) {
		int x1 = rand() % mapWidth;
		int y1 = rand() % mapHeight;
		if(game->board.safearea[x1][y1] && !game->board.powerup[x1][y1]) {
			*x = x1 * tileSize + tileSize/2;
			*y = y1 * tileSize + tileSize/2;
			return 1;
		}
	}
	return 0;
}

