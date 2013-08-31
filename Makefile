ifdef WIN
  CFLAGS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --cflags` -I/opt/SDL-1.2.13/include/ -I/opt/SDL-1.2.13/include/SDL/ -O9 -DRELEASE
  LIBS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf  -mconsole
  # CC=i586-mingw32msvc-gcc
  CC=i386-mingw32-gcc
  OUTPUT=hardheaded.exe
else
  CFLAGS=`sdl-config --cflags` -DRELEASE # -ggdb -pg
  LIBS=`sdl-config --libs` -L/opt/local/lib -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf -lm # -pg
  CC=gcc
  OUTPUT=hardheaded
endif

.PHONY: all clean depend

all: depend $(OUTPUT)

clean:
	rm -fv hardheaded hardheaded.exe *.o .depend gmon.out

depend: .depend

.depend: $(wildcard *.c)
	$(CC) $(CFLAGS) -MM $^ > .depend

include .depend

$(OUTPUT): hardheaded.o keyboard.o font.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

