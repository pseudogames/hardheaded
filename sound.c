#include "sound.h"

Mix_Music *music = NULL;
Mix_Chunk *indy_die_ptr;
Mix_Chunk *allan_die_ptr;
Mix_Chunk *special_ptr;
Mix_Chunk *attack_ptr[5];
Mix_Chunk *wakeup_ptr[3];

char* lastMusic = "";
Audio a;

void sound_terminate()
{
	//Mix_Quit();
}

void playMusic(const char* path, int qtd){
  char name[300];

  sprintf(name, "data/%s", path);
  if(lastMusic == name){
	return;
  }

  if(lastMusic != ""){
	halt_music();
  }

  lastMusic = name;
  //printf("Name: %s\n", name);
  music = Mix_LoadMUS(name);

  if(Mix_PlayMusic(music, qtd) == -1) {
	printf("Mix_PlayMusic: %s\n", Mix_GetError());
	// well, there's no music, but most games don't break without music...
  }
}

void playSound(Mix_Chunk *sound){
  Mix_PlayChannel(1, sound, 0);
}

void loadSFX(){
  indy_die_ptr = Mix_LoadWAV("data/wilhelm_scream.ogg");
  allan_die_ptr = Mix_LoadWAV("data/howie-scream.ogg");
  special_ptr = Mix_LoadWAV("data/special_whip.ogg");
  wakeup_ptr[0] = Mix_LoadWAV("data/wakeup_1.ogg");
  wakeup_ptr[1] = Mix_LoadWAV("data/wakeup_2.ogg");
  wakeup_ptr[2] = Mix_LoadWAV("data/wakeup_3.ogg");
  attack_ptr[0] = Mix_LoadWAV("data/whip_1.ogg");
  attack_ptr[1] = Mix_LoadWAV("data/whip_2.ogg");
  attack_ptr[2] = Mix_LoadWAV("data/whip_3.ogg");
  attack_ptr[3] = Mix_LoadWAV("data/whip_4.ogg");
  attack_ptr[4] = Mix_LoadWAV("data/whip_5.ogg");
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
