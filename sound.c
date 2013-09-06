#include "sound.h"

#include "data/wilhelm_scream.h"
#include "data/howie-scream.h"
#include "data/special_whip.h"
#include "data/wakeup_1.h"
#include "data/wakeup_2.h"
#include "data/wakeup_3.h"
#include "data/whip_1.h"
#include "data/whip_2.h"
#include "data/whip_3.h"
#include "data/whip_4.h"
#include "data/whip_5.h"
#include "data/zombie_1.h"
#include "data/zombie_2.h"
#include "data/zombie_3.h"
#include "data/bite.h"

Mix_Music *music = NULL;
Mix_Chunk *indy_die_ptr;
Mix_Chunk *bite_ptr;
Mix_Chunk *allan_die_ptr;
Mix_Chunk *special_ptr;
Mix_Chunk *attack_ptr[5];
Mix_Chunk *wakeup_ptr[3];
Mix_Chunk *wave_ptr[3];

const void* lastMusic = NULL;
Audio a;

void sound_terminate()
{
	//Mix_Quit();
}

void playMusic(const void* mem, int len, int qtd){
  if(lastMusic == mem){
	return;
  }
  if(lastMusic != NULL){
	halt_music();
  }

  lastMusic = mem;
  music = Mix_LoadMUS_RW(SDL_RWFromConstMem(mem,len));

  if(Mix_PlayMusic(music, qtd) == -1) {
	printf("Mix_PlayMusic: %s\n", Mix_GetError());
	// well, there's no music, but most games don't break without music...
  }
}

void playSound(Mix_Chunk *sound){
  Mix_PlayChannel(-1, sound, 0);
}

void loadSFX(){
  indy_die_ptr  = Mix_LoadWAV_RW(SDL_RWFromConstMem(wilhelm_scream_ogg, wilhelm_scream_ogg_len),0);
  allan_die_ptr = Mix_LoadWAV_RW(SDL_RWFromConstMem(howie_scream_ogg  , howie_scream_ogg_len),0);
  bite_ptr= Mix_LoadWAV_RW(SDL_RWFromConstMem(bite_ogg  , bite_ogg_len),0);
  special_ptr   = Mix_LoadWAV_RW(SDL_RWFromConstMem(special_whip_ogg  , special_whip_ogg_len),0);
  wakeup_ptr[0] = Mix_LoadWAV_RW(SDL_RWFromConstMem(wakeup_1_ogg      , wakeup_1_ogg_len),0);
  wakeup_ptr[1] = Mix_LoadWAV_RW(SDL_RWFromConstMem(wakeup_2_ogg      , wakeup_2_ogg_len),0);
  wakeup_ptr[2] = Mix_LoadWAV_RW(SDL_RWFromConstMem(wakeup_3_ogg      , wakeup_3_ogg_len),0);
  attack_ptr[0] = Mix_LoadWAV_RW(SDL_RWFromConstMem(whip_1_ogg        , whip_1_ogg_len),0);
  attack_ptr[1] = Mix_LoadWAV_RW(SDL_RWFromConstMem(whip_2_ogg        , whip_2_ogg_len),0);
  attack_ptr[2] = Mix_LoadWAV_RW(SDL_RWFromConstMem(whip_3_ogg        , whip_3_ogg_len),0);
  attack_ptr[3] = Mix_LoadWAV_RW(SDL_RWFromConstMem(whip_4_ogg        , whip_4_ogg_len),0);
  attack_ptr[4] = Mix_LoadWAV_RW(SDL_RWFromConstMem(whip_5_ogg        , whip_5_ogg_len),0);
  wave_ptr[1] = Mix_LoadWAV_RW(SDL_RWFromConstMem(zombie_1_ogg        , zombie_1_ogg_len),0);
  wave_ptr[2] = Mix_LoadWAV_RW(SDL_RWFromConstMem(zombie_2_ogg        , zombie_2_ogg_len),0);
  wave_ptr[3] = Mix_LoadWAV_RW(SDL_RWFromConstMem(zombie_3_ogg        , zombie_3_ogg_len),0);
}

void playIndyDeath(){
  playSound(indy_die_ptr);
}

void playAllanDeath(){
  playSound(allan_die_ptr);
}

void playWakeup(){
  playSound(wakeup_ptr[rand() %  3]);
}

void playBite(){
  playSound(bite_ptr);
}


void playNewWave(){
  playSound(wave_ptr[rand() %  3]);
}

void playAttack(){
  playSound(attack_ptr[rand() %  5]); 
}

void playSpecialAttack(){
  playSound(special_ptr);
}

void soundInit() {
  a.rate = 22050;
  a.format = AUDIO_S16; /* 16-bit stereo */
  a.channels = 2;
  a.buffers = 4046;

  if(Mix_OpenAudio(a.rate, a.format, a.channels, a.buffers)) {
	printf("Unable to open audio!\n");
	exit(1);
  }

  Mix_QuerySpec(&a.rate, &a.format, &a.channels);
}

void halt_music(){
  Mix_HaltMusic();
  Mix_FreeMusic(music);
  music = NULL;
  lastMusic = "";
}
