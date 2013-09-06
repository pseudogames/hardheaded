#ifndef _FONT_H
#define _FONT_H
#include <SDL_ttf.h>

#define DEFAULT_PTSIZE 18
#define DEFAULT_TEXT "The quick brown fox jumped over the lazy dog"
#define NUM_COLORS 256


void init_font();
void terminate_font();
void text_write(SDL_Surface *screen, int x, int y, char *term, int selected);
void text_write_raw_ttf(SDL_Surface *screen, int x, int y, char *message, SDL_Color color, int points, const void* mem, int len);
void text_write_raw(SDL_Surface *screen, int x, int y, char *message, SDL_Color color, int points);
#endif
