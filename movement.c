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
	int hit;
	x/=tileSize;
	y/=tileSize;

	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		hit = 1;
	else if((int)body->pos.x/tileSize == x && (int)body->pos.y/tileSize == y)
		hit = 0;
	else 
		hit = game->board.crowd[x][y];

	//printf("solid:%d, %d %d %d %d\n",hit, (int)body->pos.x/tileSize , x , (int)body->pos.y/tileSize , y);

	return hit;
}

inline int is_empty(Game *game, Body *body, int x, int y)
{
	return !is_solid(game, body,x,y);
}

inline int is_air(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;
	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		return 1;
	return !game->board.air[x][y];
}

inline int is_hit(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;
	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		return 1;
	return game->board.hittable[x][y];
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

void moveBody(Game *game, Body *body, float angle, float moving)
{
	float v = body->vel * moving;

	angle_rotate(&body->angle, angle, body->ang_vel);
#define ANGLE_STEP 15
	int a0 = round(body->angle / ANGLE_STEP) * ANGLE_STEP;
//	fprintf(stderr, "%f %f\n", body->angle, a0);
	float a = a0 * M_PI / 180.;
	float dx = cos(a) * v;
	float dy = sin(a) * v;
	float x0 = body->pos.x;
	float y0 = body->pos.y;
	float x1 = x0 + dx;
	float y1 = y0 - dy;


	int mx = (int)x0/TILESIZE - (int)x1/TILESIZE;
	int my = (int)y0/TILESIZE - (int)y1/TILESIZE;
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

void movePlayer(App *app, Player *player, int up, int right, int down, int left, int halt)
{
	Body *body = &player->body;
	int hit = 0;

	if(body->action == ACTION_DEATH) return;

	if(player->grabbing){
		float a = player->body.angle * M_PI / 180;
		float dx = cos(a) * HOLD_DISTANCE;
		float dy = sin(a) * HOLD_DISTANCE;
		float tx = body->pos.x + dx;
		float ty = body->pos.y - dy;
#if 0
		if(!is_empty(&app->game, &app->game.head.body, tx,ty)) {
			dx = cos(a) * HOLD_DISTANCE/2;
			dy = sin(a) * HOLD_DISTANCE/2;
			tx = body->pos.x + dx;
			ty = body->pos.y - dy;
		}
#endif
		if(!is_empty(&app->game, &app->game.head.body, tx,ty)) {
			// player->grabbing = 0;
		} else {
			app->game.head.body.pos.x = tx;
			app->game.head.body.pos.y = ty;
			app->game.head.body.angle = body->angle;
			body->vel = 1;
		}
	} 
	if(!player->grabbing) {
		body->vel = body->max_vel;
	}

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
        moveBody(&app->game, body, angle, !halt && !hit);
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
			if(app->game.board.enemies[i].body.life <= 0) {
				app->game.board.enemies[i].alive = 0;
				continue;
			}
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
				app->game.indy.door.x = x * tileSize + tileSize/2;
				app->game.indy.door.y = y * tileSize + tileSize/2;
				app->game.indy.body.angle = 90; // FIXME look to the idol head
			}
			if(p2) {
				app->game.allan.body.pos.x = x * tileSize + tileSize/2;
				app->game.allan.body.pos.y = y * tileSize + tileSize/2;
				app->game.allan.door.x = x * tileSize + tileSize/2;
				app->game.allan.door.y = y * tileSize + tileSize/2;
				app->game.allan.body.angle = 270; // FIXME look to the idol head

				app->game.head.body.pos.x = app->game.allan.body.pos.x + 40;
				app->game.head.body.pos.y = app->game.allan.body.pos.y;
			}
			if(head) {
				app->game.head.body.pos.x = x * tileSize + tileSize/2;
				app->game.head.body.pos.y = y * tileSize + tileSize/2;
				app->game.head.body.angle = 270;

			}
		}
	}
	movePrepare(app);
}

void moveEnemies(App *app)
{
  if(app->game.winner) return;
  int i;
  int t = SDL_GetTicks();
  for(i=0; i < ENEMY_COUNT; i++)
  {
    if(app->game.board.enemies[i].alive
	&& app->game.board.enemies[i].body.life <= 0) {
		app->game.board.enemies[i].alive = 0;
	}
  }

  //printf("-------------\n");
  int n=AI_PER_FRAME;
  for(i=0; i < ENEMY_COUNT && n > 0; i++) 
  {
    int id = app->game.board.latest_enemy_updated = ( app->game.board.latest_enemy_updated + 1 ) % ENEMY_COUNT;
    int crazy = id*3;
    if(app->game.board.enemies[id].alive && t > app->game.board.enemies[id].last_ai+AI_FREQ)
    {
        Body *enemy_body = &app->game.board.enemies[id].body;
		n--;
		//printf("path %d %d %d\n", i, n, id);
		app->game.board.enemies[id].last_ai = t;
		pathLength[crazy] = 9999;
		pathLength[crazy+1] = 9999;
		pathLength[crazy+2] = 9999;

		if(app->game.indy.body.action != ACTION_DEATH) {
			//printf("find %d=%d\n", id, crazy);
			pathStatus[crazy] = FindPath(crazy,
				enemy_body->pos.x,
				enemy_body->pos.y,
				app->game.indy.body.pos.x,
				app->game.indy.body.pos.y);
		}

		if(app->game.allan.body.action != ACTION_DEATH) {
			//printf("find %d=%d\n", id, crazy+1);
			pathStatus[crazy+1] = FindPath(crazy+1,
				enemy_body->pos.x,
				enemy_body->pos.y,
				app->game.allan.body.pos.x,
				app->game.allan.body.pos.y);
		}

		if(app->game.allan.body.action == ACTION_DEATH &&
		   app->game.indy.body.action == ACTION_DEATH) {
			//printf("find %d=%d\n", id, crazy+2);
			pathStatus[crazy+2] = FindPath(crazy+2,
				enemy_body->pos.x,
				enemy_body->pos.y,
				app->game.head.body.pos.x,
				app->game.head.body.pos.y);
		}


        if(pathLength[crazy] < pathLength[crazy+1] && pathLength[crazy] < pathLength[crazy+2]){
          app->game.board.enemies[id].pathfinder = crazy;
          app->game.board.enemies[id].pathfinder_other = crazy+1;
          app->game.board.enemies[id].pathfinder_another = crazy+2;
          app->game.board.enemies[id].target = &app->game.indy;
        } else if(pathLength[crazy+1] < pathLength[crazy+2]) {
          app->game.board.enemies[id].pathfinder = crazy+1;
          app->game.board.enemies[id].pathfinder_other = crazy;
          app->game.board.enemies[id].pathfinder_another = crazy+2;
          app->game.board.enemies[id].target = &app->game.allan;
        }else{
          app->game.board.enemies[id].pathfinder = crazy+2;
          app->game.board.enemies[id].pathfinder_other = crazy;
          app->game.board.enemies[id].pathfinder_another = crazy+1;
          app->game.board.enemies[id].target = &app->game.head;
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
    if(app->game.board.enemies[i].alive)
    {
        Body *enemy_body = &app->game.board.enemies[i].body;
		int crazy = app->game.board.enemies[i].pathfinder;
        if(pathStatus[crazy] == found)
        {
			//printf("read %d=%d\n", i, crazy);
          int reach = ReadPath(crazy, enemy_body->pos.x, enemy_body->pos.y, tileSize);
          int dx = xPath[crazy] - enemy_body->pos.x;
          int dy = yPath[crazy] - enemy_body->pos.y;
          //float angle = ATAN2(dx,dy);
          float angle = (int)(720 + ATAN2(dx,dy) + sin((t/5000.+crazy/(float)AI_PER_FRAME)*M_PI)*30 ) % 360;
          moveBody(&app->game, enemy_body, angle, .25+.75*rand()/(float)RAND_MAX);
		  //printf("enemy %d (%f, %f) angle %f -> %f\n", crazy, enemy_body->pos.x, enemy_body->pos.y, angle, enemy_body->angle);

			  
		  if(reach && (t > app->game.board.enemies[i].target->last_ai+HIT_FREQ)){
			  app->game.board.enemies[i].target->last_ai = t;
			  //printf("reach %d=%d %d,%d\n", i, crazy, dx, dy);
			  pathStatus[app->game.board.enemies[i].pathfinder_other] = notStarted;
			  pathStatus[app->game.board.enemies[i].pathfinder_another] = notStarted;
			  hit(app, enemy_body, &app->game.board.enemies[i].target->body);

		  }
		}
	}
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


