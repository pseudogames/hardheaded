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
	if(player->body.life <= 10 && player->body.action == ACTION_DEATH){
		player->body.life += 0.05;
		if(player->body.life > 10) player->body.life = 10;
	}
}

void renderEnemies(App *app)
{
  int i = 0;
  for(; i < ENEMY_COUNT; i++) {
    if(app->game.board.enemies[i].alive)
	  renderBody(app, &app->game.board.enemies[i].body);
  }
}


void renderGameplay(App *app){
	SDL_BlitSurface(app->game.board.image, NULL, app->screen, NULL);

	renderDebug(app);

	renderLifeBars(app);
	renderEnemies(app);
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

	//printf("Deltas, f: %f\n", d);


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
	player->body.life = 0;
	Body *body = &player->body;
	body->action = ACTION_DEATH;
	body->frame = 0;
}

void spawnEnemy(App *app)
{
	Game *game = &app->game;
	int x,y,i;
	Wave *wave = &game->wave[game->board.wave_index];
	int t = SDL_GetTicks();

	// printf("spawn tick %d\n", t);
	//if(game->on_screen_enemies>0 && t < app->game.spawnTime)
	if(t < app->game.spawnTime)
		return;
	// printf("spawn started %d\n", t);

	int spawnDelay = wave->enemy_spawn_interval;

	int spawn = (int)ceil(wave->enemy_count_per_spawn/2.) 
		+ (rand() % (int)ceil(wave->enemy_count_per_spawn/2.));

	if(spawn + game->on_screen_enemies > wave->enemy_count_on_screen) {
		// printf("spawn %d on_screen\n", game->on_screen_enemies);
		spawn = wave->enemy_count_on_screen - game->on_screen_enemies;
	}
	
	if(spawn + game->total_enemies > wave->enemy_count) {
		// printf("spawn %d wave\n", game->total_enemies);
		spawn = wave->enemy_count - game->total_enemies;
	}

	// printf("spawn %d enemies\n", spawn);
	while(spawn > 0) {
		Enemy *enemy = NULL;
		for(i = 0; i < wave->enemy_count; i++)
		{
			if(!game->board.enemies[i].alive)
			{
				game->total_enemies ++;
				game->on_screen_enemies ++;
				spawn --;
				enemy = &game->board.enemies[i];
				break;
			}
		}

		if(enemy != NULL && enemy_spawn_pos(game, &x,&y))
		{
			// printf("spawn %d\n", i);
			Body *enemybody = &enemy->body;
			enemybody->life = 1;
			enemybody->ang_vel = 0.05;
			enemybody->max_vel = 2;
			enemybody->vel = 2;
			enemybody->angle = 270;
			enemybody->pos.x = x;
			enemybody->pos.y = y;
			enemybody->sprite = &app->zombie;
			enemy->variation = rand() % app->enemy_variation_count;
			enemy->alive = 1;
		} else {
			spawnDelay = 500;
			break; // did not finished group spawn, persist
		}
	}
	app->game.spawnTime = t + spawnDelay;
}

int hit(App *app, Body *source, Body *target){
#if 0
	if(target == NULL || source == NULL) {
		return 0;
	}

	float dist = sqrt(
		pow(target->pos.x - source->pos.x,2)+
		pow(target->pos.y - source->pos.y,2)
	);

	target->life -= 1;

	if(target->life > 0){
		playSound(target->onHitSound);
	}

	if(target->life <= 0){
		target->life = 0;
		if(target->status == BODY_ALIVE){
			int enemy_killed = 
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
				Wave *wave = &app->game.board.wave[app->game.board.wave_index];
				if(enemy_killed) {
					app->game.on_screen_enemies --;
					if(target->exploded) {
						app->game.total_enemies --;
					} else {
						app->game.kill_count ++;
						app->game.total_kill_count ++;
						int score = target->item.type->score;
						for(i=0;i<21;i++) {
							int x = x0+search[i][0];
							int y = y0+search[i][1];
							if( x < 1 || y < 1 || x >= wave->w-1 || y >= wave->h-1 || walkability[x][y]==1) 
								continue; // dont outside or on the enemy spawn borders
							int s = score * ceil( (21-i) / 4.);
							app->game.board.death1[x][y] += s;
							app->game.board.death2[x][y] += s;
						}
					}

				} else { // player_killed
					int x1 = source->pos.x/tileSize;
					int y1 = source->pos.y/tileSize;
					for(i=0;i<21;i++) {
						int x = x1+search[i][0];
						int y = y1+search[i][1];
						if( x < 1 || y < 1 || x >= wave->w-1 || y >= wave->h-1 || walkability[x][y]==1) 
							continue; // dont outside or on the enemy spawn borders
						app->game.board.death1[x][y] /= 2;
					}
					for(i=0;i<21;i++) {
						int x = x0+search[i][0];
						int y = y0+search[i][1];
						if( x < 1 || y < 1 || x >= wave->w-1 || y >= wave->h-1 || walkability[x][y]==1) 
							continue; // dont outside or on the enemy spawn borders
						app->game.board.death1[x][y] /= 2;
					}
				}
				target->status = BODY_DEAD;
				int index = app->game.board.wave_index;
				int count = app->game.board.wave[app->game.board.wave_index].enemy_count;
				if(count == app->game.kill_count){
					setWave(app, index+1);    
				}
			}
		}
		return 1;
	}
	return 0;
#endif
}

