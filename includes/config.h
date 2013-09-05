#ifndef _CONFIG_H
#define _CONFIG_H

#include <SDL.h>

typedef struct {
	SDLKey start;
	SDLKey up;
	SDLKey down;
	SDLKey left;
	SDLKey right;
	SDLKey attack;
} KeyBindPlayer;

typedef struct {
	KeyBindPlayer player1;
	KeyBindPlayer player2;
} KeyBind;

extern KeyBind keybind;

void conf_keys_load(char *path);
void conf_keys_write(char *path);

#endif // _CONFIG_H
