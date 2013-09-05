ifdef WIN
  CFLAGS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --cflags` -I/opt/SDL-1.2.13/include/ -I/opt/SDL-1.2.13/include/SDL/ -O9 -DRELEASE
  LIBS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf  -mconsole
  # CC=i586-mingw32msvc-gcc
  CC=i386-mingw32-gcc
  OUTPUT=hardheaded.exe
else
  CFLAGS=`sdl-config --cflags` -Iincludes -ggdb #-pg
  LIBS=`sdl-config --libs` -L/opt/local/lib -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf -lm # -pg
  CC=gcc
  OUTPUT=hardheaded
endif

OBJS=hardheaded.o render.o keyboard.o font.o menu.o gameplay.o sprite.o movement.o aStarLibrary.o sound.o config.c

INCS=data/all.h

.PHONY: all clean depend

all: $(OUTPUT)

clean:
	# make -C data clean
	rm -fv hardheaded hardheaded.exe *.o .depend gmon.out

data/all.h:
	make -C $$(dirname "$@") all

iniparser/libiniparser.a:
	make -C $$(dirname "$@")

depend: .depend

.depend: $(patsubst %.o,%.c,$(OBJS)) | data/all.h
	$(CC) $(CFLAGS) -MM $^ > .depend

ifneq ($(MAKECMDGOALS),clean)
	-include .depend
endif

$(OUTPUT): $(INCS) $(OBJS) iniparser/libiniparser.a
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

