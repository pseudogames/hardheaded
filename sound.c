#include "sound.h"

Mix_Music *music = NULL;
char* lastMusic = "";
Audio a;
const char* levelMusics[3] = {"ascending.mp3", "arpanauts.mp3", "underclocked.mp3"};

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

void playRandomMusic(){
  const char* musicName = levelMusics[rand()%3];
  playMusic(musicName, -1);
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
