#include "gameplay.h"
#include <math.h>
#include "sound.h"

void renderLifeBars(App *app){
	SDL_Surface *screen = app->screen;
	Board *board = &app->game.board;

	renderPlayerSpecialBar(app, screen, &app->game.indy, 0);
	renderPlayerSpecialBar(app, screen, &app->game.allan, 650);

	renderPlayerLife(app, screen, &app->game.indy, 0);
	renderPlayerLife(app, screen, &app->game.allan, 650);

	renderSpawnCountdown(app);
}

void checkPlayerLife(Player *player, App *app){
	if(player->body.life > PLAYER_HEALTH)
		player->body.life = PLAYER_HEALTH;
	if(app->game.winner){
		if(player != app->game.winner){
			player->body.action = ACTION_DEATH;
		
			if(player->body.life > 0){
				player->body.life -= 0.08;
			}
		}
	} else if(player->body.action == ACTION_DEATH && app->game.head.body.life > 0){
		player->grabbing = 0;
		player->special_attack = 0;

		if( player->body.life <= PLAYER_HEALTH ) {
			player->body.life += 0.05;
			if(player->body.life > PLAYER_HEALTH) {
				player->body.life = PLAYER_HEALTH;
				player->body.action = ACTION_MOVE;
			}
		}
	} else if(player->body.life <= 0) {
		playerDie(app, player);
	}
}

void renderEnemies(App *app)
{
  int i = 0;
  for(; i < ENEMY_COUNT; i++) {
    if(app->game.board.enemies[i].alive){
		if(app->game.winner){
			app->game.board.enemies[i].body.action = ACTION_DEATH;
			app->game.board.enemies[i].body.frame += 0.3;
		}

		if(app->game.board.enemies[i].body.action != ACTION_DEATH) {
			float frame = app->game.board.enemies[i].body.frame;
			
			frame += 0.1;

			if(frame < app->enemy_frame_count * app->game.board.enemies[i].variation
			|| frame >= app->enemy_frame_count * (app->game.board.enemies[i].variation+1) ) {
				app->game.board.enemies[i].body.action = ACTION_MOVE;
				frame = app->enemy_frame_count * app->game.board.enemies[i].variation;
			}

			app->game.board.enemies[i].body.frame = frame;
		}

		renderBody(app, &app->game.board.enemies[i].body);
	}
  }
}

void renderGameplay(App *app){
	SDL_BlitSurface(app->game.board.image, NULL, app->screen, NULL);

	renderDebug(app);

	if(app->game.winner){
		app->game.winner->grabbing = 0;
	}

	if(app->game.head.body.life <= 0){
		app->game.allan.body.action = ACTION_DEATH;
		app->game.indy.body.action = ACTION_DEATH;
		renderGameOver(app);
	}

	if(app->game.winner){
		renderWinner(app);
	}
	renderLifeBars(app);
	renderEnemies(app);
	renderPlayer(app, &app->game.indy);
	renderPlayer(app, &app->game.allan);
	renderHead(app);
	renderHints(app);

	checkPlayerLife(&app->game.indy, app);
	checkPlayerLife(&app->game.allan, app);

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

	//printf("Deltas, f: %f\n", d);


	if( d < PICK_DISTANCE ) return 1;
	return 0;
}

void playerChargeSpecialAttack(App *app, Player *player){
	Body *body = &player->body;
	Body *head_body = &app->game.head.body;

	if(!player->grabbing && near(body, head_body) && player->special_attack < 7 && !app->game.winner && !app->game.head.body.life <= 0){
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
			float dist = sqrt(
				pow(app->game.head.body.pos.x - player->door.x, 2)+
				pow(app->game.head.body.pos.y - player->door.y, 2)
			);


			player->grabbing = 1;
			head_body->action = ACTION_ATTACK;

			if(dist < tileSize*1.5){
				app->game.winner = player;
				player->grabbing = 0;
			}
		}
	}

	if(!player->grabbing && player->special_attack < 100 && app->game.head.body.life > 0) {
		player->special_attack += 5;
		player->power_body.action = ACTION_MOVE;
	}

}

void playerAttack(App *app, Player *player){
	int i;
	Body *body = &player->body;
	Body *head_body = &app->game.head.body;

	if(body->action == ACTION_DEATH) return;
	if(player->grabbing){
		head_body->action = ACTION_MOVE;
		player->grabbing = 0;
		return;
	}

	if(player->special_attack >= 100) {
		player->power_body.action = ACTION_ATTACK;
		playSpecialAttack();
	}else{
		playAttack();
	}

	body->action = ACTION_ATTACK;
	body->frame = 0;

	shoot(app, &player->body, 10, 0, tileSize*1.5);
	shoot(app, &player->body, 50, 0, tileSize*1);
	shoot(app, &player->body, 100, 180, tileSize*.5);

	player->special_attack = 0;
}

void playerDie(App *app, Player *player){
	player->body.life = 0;
	Body *body = &player->body;
	body->action = ACTION_DEATH;
	body->frame = 0;
	if(&app->game.allan == player){
		playAllanDeath();
	}else if(&app->game.indy == player){
		playIndyDeath();
	}
}

void spawnEnemy(App *app)
{
	Game *game = &app->game;
	int x,y,i;

	if(game->wave_index < 0) return;

	Wave *wave = &game->wave[game->wave_index];
	int t = SDL_GetTicks();


	// printf("spawn tick %d\n", t);
	//if(game->on_screen_enemies>0 && t < app->game.spawnTime)
	if(t < app->game.board.spawnTime)
		return;
	// printf("spawn started %d\n", t);

	int spawnDelay = wave->enemy_spawn_interval;

	int spawn = (int)ceil(wave->enemy_count_per_spawn/2.) 
		+ (rand() % (int)ceil(wave->enemy_count_per_spawn/2.));

	if(spawn + game->board.on_screen_enemies > wave->enemy_count_on_screen) {
		// printf("spawn %d on_screen\n", game->on_screen_enemies);
		spawn = wave->enemy_count_on_screen - game->board.on_screen_enemies;
	}
	
	if(spawn + game->board.total_enemies > wave->enemy_count) {
		// printf("spawn %d wave\n", game->total_enemies);
		spawn = wave->enemy_count - game->board.total_enemies;
	}

	// printf("spawn %d enemies\n", spawn);
	while(spawn > 0) {
		Enemy *enemy = NULL;
		for(i = 0; i < wave->enemy_count; i++)
		{
			if(!game->board.enemies[i].alive)
			{
				game->board.total_enemies ++;
				game->board.on_screen_enemies ++;
				spawn --;
				enemy = &game->board.enemies[i];
				break;
			}
		}

		if(enemy != NULL && enemy_spawn_pos(game, &x,&y))
		{
			// printf("spawn %d\n", i);
			Body *enemybody = &enemy->body;
			enemybody->score = ZOMBIE_SCORE;
			enemybody->life = ZOMBIE_HEALTH;
			enemybody->kills = 0;
			enemybody->ang_vel = 0.05;
			enemybody->max_vel = 2;
			enemybody->vel = 2;
			enemybody->angle = 270;
			enemybody->pos.x = x;
			enemybody->pos.y = y;
			enemybody->sprite = &app->zombie;
			enemybody->action = ACTION_MOVE;
			enemy->variation = rand() % app->enemy_variation_count;
			enemybody->frame = enemy->variation * app->enemy_frame_count;
			enemy->alive = 1;
		} else {
			spawnDelay = 500;
			break; // did not finished group spawn, persist
		}
	}
	app->game.board.spawnTime = t + spawnDelay;
}

int hit(App *app, Body *source, Body *target){
	if(target == NULL || source == NULL) {
		return 0;
	}

	float dist = sqrt(
		pow(target->pos.x - source->pos.x,2)+
		pow(target->pos.y - source->pos.y,2)
	);

	float damage = DAMAGE;
	int alive = target->life > 0;
	target->life -= damage;

	if(target->life > 0){
		//playSound(target->onHitSound);
	}

	if(target->life <= 0){
		target->life = 0;
		if(alive){
			int enemy_killed = !!target->score;
			if(!app->debug || enemy_killed) { // player immortal on debug
				int x0 = target->pos.x/tileSize;
				int y0 = target->pos.y/tileSize;
				int search[21][2] = {
					{+0,+0},
					{+1,+0},
					{+0,+1},
					{-1,+0},
					{+0,-1},
					{+1,-1},
					{+1,+1},
					{-1,+1},
					{-1,-1},
					{+0,-2},
					{+2,+0},
					{+0,+2},
					{-2,+0},
					{-1,-2},
					{+1,-2},
					{+2,-1},
					{+2,+1},
					{+1,+2},
					{-1,+2},
					{-2,+1},
					{-2,-1}
				};

				int i;
				Wave *wave = &app->game.wave[app->game.wave_index];
				if(enemy_killed && target->action != ACTION_DEATH) {
					app->game.board.on_screen_enemies --;
					app->game.board.kill_count ++;
					app->game.total_kill_count ++;
					source->kills++;
					float score = target->score;
					source->life += score;
					for(i=0;i<21;i++) {
						int x = x0+search[i][0];
						int y = y0+search[i][1];
						if( x < 0 || y < 0 || x >= mapWidth || y >= mapHeight || walkability[x][y]==1) 
							continue; // dont outside or on the enemy spawn borders
						int s = score * ceil( (21-i) / 4.);
						app->game.board.death1[x][y] += s;
						app->game.board.death2[x][y] += s;
					}

				} 
				if(!enemy_killed && target->action != ACTION_DEATH) {// player_killed
					int x1 = source->pos.x/tileSize;
					int y1 = source->pos.y/tileSize;
					for(i=0;i<21;i++) {
						int x = x1+search[i][0];
						int y = y1+search[i][1];
						if( x < 0 || y < 0 || x >= mapWidth || y >= mapHeight || walkability[x][y]==1) 
							continue; // dont outside or on the enemy spawn borders
						app->game.board.death1[x][y] /= 2;
					}
					for(i=0;i<21;i++) {
						int x = x0+search[i][0];
						int y = y0+search[i][1];
						if( x < 0 || y < 0 || x >= mapWidth || y >= mapHeight || walkability[x][y]==1) 
							continue; // dont outside or on the enemy spawn borders
						app->game.board.death1[x][y] /= 2;
					}
				}

#if 0
				int index = app->game.board.wave_index;
				int count = app->game.wave[app->game.board.wave_index].enemy_count;
				if(count == app->game.board.kill_count){
					setWave(app, index+1);    
				}
#endif
			}
		}
		return 1;
	}
	return 0;
}

inline int draw(App *app, Body *body, int x, int y)
{

#if 1
	SDL_Rect rect = { x-1, y-1, 3, 3 };
	Uint32 color = SDL_MapRGB(app->screen->format, 0xFF, 0xFF,0x00 );
	SDL_FillRect(app->screen, &rect, color);
	//if(rand()%500 == 0) SDL_Flip(app->screen);
#endif

	int target = is_hit(&app->game, body, x, y);
	// printf("hit %d,%d: %d\n", x,y, target);

	if(target==2 && app->game.indy.body.action != ACTION_DEATH) {
		hit(app, body, &app->game.indy.body);
	}
	if(target==3 && app->game.allan.body.action != ACTION_DEATH) {
		hit(app, body, &app->game.allan.body);
	}
	if(target>=4) {
		int idx = target - 4;
		hit(app, body, &app->game.board.enemies[idx].body);
	}

	return target;
}

int shoot(App *app, Body *body, int spread, int angle, float range)
{
	int x0, y0;
	int x1, y1, x2, y2;
	int dx, dy, i, e;
	int incx, incy, inc1, inc2;
	int x,y;

	if(body->action == ACTION_DEATH)
		return 0;

	//playSound(body->item.type->sound, -1);

	x0 = body->pos.x;
	y0 = body->pos.y;

	float a = (int)(body->angle + angle + ((rand() % (spread+1)) - spread/2))%360;

	dx = + cos(a * M_PI / 180.) * range;
	dy = - sin(a * M_PI / 180.) * range;

	x1 = x0 - dx * 0.2;
	y1 = y0 - dy * 0.2;

	x2 = x0 + dx;
	y2 = y0 + dy;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;
	incx = 1;
	if(x2 < x1) incx = -1;
	incy = 1;
	if(y2 < y1) incy = -1;
	x=x1;
	y=y1;

	if(dx > dy)
	{
	  if(draw(app,body,x,y)) return 1;
      e = 2*dy - dx;
      inc1 = 2*( dy -dx);
      inc2 = 2*dy;
      for(i = 0; i < dx; i++)
      {
        if(e >= 0)
        {
          y += incy;
          e += inc1;
        } else e += inc2;
        x += incx;
        if(draw(app,body,x,y)) return 1;
      }
  }
  else
  {
    if(draw(app,body,x,y)) return 1;
      e = 2*dx - dy;
      inc1 = 2*( dx - dy);
      inc2 = 2*dx;
      for(i = 0; i < dy; i++)
      {
        if(e >= 0)
        {
          x += incx;
          e += inc1;
        }
        else e += inc2;
        y += incy;
        if(draw(app,body,x,y)) return 1;
      }
  }

  return 0;
}

