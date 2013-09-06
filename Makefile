ifdef WIN
  export CC=/tmp/mxe/usr/bin/i686-pc-mingw32-gcc
  SDLCONFIG=/tmp/mxe/usr/bin/i686-pc-mingw32-sdl-config
  PREFIX=/tmp/mxe/usr/i686-pc-mingw32/
  CFLAGS=`$(SDLCONFIG) --cflags` -Iincludes -I$(PREFIX)/include -O9 -DRELEASE
  LIBS=`$(SDLCONFIG) --libs` -I$(PREFIX)/lib -lSDL_ttf -lfreetype -lSDL_gfx -lSDL_image -ljpeg -lpng -lSDL_mixer -lvorbisfile -lvorbis -logg -lmikmod -lmodplug -lsmpeg -lSDL -lwinmm -lbz2 -lz -lstdc++ -lm -mconsole
  OUTPUT=hardheaded.exe
else
  CFLAGS=`sdl-config --cflags` -Iincludes -ggdb #-pg
  LIBS=`sdl-config --libs` -L/opt/local/lib -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf -lm # -pg
  OUTPUT=hardheaded
endif

OBJS=hardheaded.o render.o keyboard.o font.o menu.o gameplay.o sprite.o movement.o aStarLibrary.o sound.o config.c

# INCS=data/all.h

.PHONY: all clean depend

all: $(OUTPUT)

clean:
	rm -fv hardheaded hardheaded.exe *.o .depend gmon.out
	make -C iniparser veryclean
#	make -C data clean

#data/all.h:
#	make -C $$(dirname "$@")

iniparser/libiniparser.a:
	make -C $$(dirname "$@")

depend: .depend

.depend: $(patsubst %.o,%.c,$(OBJS)) | $(INCS)
	$(CC) $(CFLAGS) -MM $^ > .depend

ifneq ($(MAKECMDGOALS),clean)
-include .depend
endif

$(OUTPUT): $(INCS) $(OBJS) iniparser/libiniparser.a
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

