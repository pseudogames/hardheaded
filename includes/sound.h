#include <SDL_mixer.h>

typedef struct{
    int rate;
    Uint16 format; /* 16-bit stereo */
    int channels;
    int buffers;
} Audio;

void playMenuMusic();
void soundInit();
void halt_music();
void playRandomLevelMusic();
void playSound();
void sound_terminate();
void loadEffects();
void playWakeup();
void playAttack();
void playSpecialAttack();
void playIndyDeath();
void playAllanDeath();