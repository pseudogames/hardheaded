#include "gameplay.h"

void renderGameplay(App *app){
	renderLifeBars(app);

	SDL_Color yellow = {0xFF, 0XFF, 0x00};
	text_write_raw(app->screen, 100, 250, "Ola mundo", yellow, 30);

	SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderLifeBars(App *app){

}
