#include "font.h"
#include "render.h"
#include "data/all.h"

#define TTF_POINT_LIMIT 99
TTF_Font *ttf_point_cache[TTF_POINT_LIMIT];

void init_font() {
    TTF_Init();
	memset(ttf_point_cache, 0, sizeof(ttf_point_cache));
}

void terminate_font()
{
	int i;
	for(i=0;i<TTF_POINT_LIMIT;i++) {
		if(ttf_point_cache[i]) {
			TTF_CloseFont(ttf_point_cache[i]);
			ttf_point_cache[i] = NULL;
		}
	}
	TTF_Quit();
}

TTF_Font *setup_ttf(int points, const void* mem, int len){
	if(points >= TTF_POINT_LIMIT) return NULL;
    TTF_Font *ttf_tmp = ttf_point_cache[points];
	if(ttf_tmp) return ttf_tmp;
    ttf_point_cache[points] = ttf_tmp = TTF_OpenFontRW(SDL_RWFromConstMem(mem,len), 0, points);

    int renderstyle = TTF_STYLE_NORMAL;
    int outline = 0;
    int kerning = 1;

    TTF_SetFontStyle(ttf_tmp, renderstyle);
    return ttf_tmp;
}

void text_write(SDL_Surface *screen, int x, int y, char *message, int selected){
    SDL_Color color;

    color = (selected) ? black: red;

    text_write_raw_ttf(screen, x, y, message, color, 72, indiana_ttf, indiana_ttf_len);
}

void text_write_raw(SDL_Surface *screen, int x, int y, char *message, SDL_Color color, int points) {
	text_write_raw_ttf(screen, x, y ,message, color, points, indiana_ttf, indiana_ttf_len);
}

void text_write_raw_ttf(SDL_Surface *screen, int x, int y, char *message, SDL_Color color, int points, const void* mem, int len) {
    TTF_Font *tmp;

    tmp = setup_ttf(points, mem, len);
    SDL_Rect dstrect;
    SDL_Surface *text;

    text = TTF_RenderText_Solid(tmp, message, color);

    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = text->w;
    dstrect.h = text->h;

    SDL_BlitSurface(text, NULL, screen, &dstrect);

    SDL_FreeSurface(text);
}

