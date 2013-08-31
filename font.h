#include <SDL_ttf.h>

#define DEFAULT_PTSIZE18
#define DEFAULT_TEXT "The quick brown fox jumped over the lazy dog"
#define NUM_COLORS      256


TTF_Font *ttf_adler_font;
TTF_Font *ttf_acid_font;

void init_font();
void terminate_font();
void text_write(SDL_Surface *screen, int x, int y, char *term, int selected);
void text_write_raw(SDL_Surface *screen, int x, int y, char *message, SDL_Color color, int points );
