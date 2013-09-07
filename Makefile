ifdef WIN
  export CC=/tmp/mxe/usr/bin/i686-pc-mingw32-gcc
  SDLCONFIG=/tmp/mxe/usr/bin/i686-pc-mingw32-sdl-config
  PREFIX=/tmp/mxe/usr/i686-pc-mingw32/
  CFLAGS=`$(SDLCONFIG) --cflags` -Iincludes -I$(PREFIX)/include -O3 -DRELEASE
  LDFLAGS=-static
  LIBS=-lSDL_ttf -lfreetype -lSDL_gfx -lSDL_image -ljpeg -lpng -lSDL_mixer -lvorbisfile -lvorbis -logg -lmodplug -lsmpeg -lbz2 -lz -lstdc++ `$(SDLCONFIG) --libs` -mconsole
  # LIBS=-lSDL_ttf -lfreetype -lSDL_gfx -lSDL_image -ljpeg -lpng -lSDL_mixer -lvorbisfile -lvorbis -logg -lmikmod -lmodplug -lsmpeg -lbz2 -lz -lstdc++ `$(SDLCONFIG) --libs` -mconsole
  OUTPUT=hardheaded.exe
else
  CC=gcc
  CFLAGS=`sdl-config --cflags` -Iincludes -ggdb #-pg
  LFDLAGS=-static 
  # LIBS=-L/opt/local/lib -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf `sdl-config --libs` -lm # -pg # linux
  LIBS=-L/opt/local/lib /opt/local/lib/libSDL_ttf.a /opt/local/lib/libfreetype.a /opt/local/lib/libSDL_gfx.a /opt/local/lib/libSDL_image.a /opt/local/lib/libjpeg.a /opt/local/lib/libpng.a /opt/local/lib/libSDL_mixer.a /opt/local/lib/libvorbisfile.a /opt/local/lib/libvorbis.a /opt/local/lib/libogg.a /opt/local/lib/libmikmod.a /opt/local/lib/libflac.a /opt/local/lib/libsmpeg.a /opt/local/lib/libbz2.a /opt/local/lib/libz.a -lstdc++ `sdl-config --static-libs` -mconsole # mac
  OUTPUT=hardheaded
endif

OBJS=hardheaded.o render.o keyboard.o font.o menu.o gameplay.o sprite.o movement.o aStarLibrary.o sound.o config.o iniparser/src/dictionary.o iniparser/src/iniparser.o

INCS=\
data/14heart_small.h \
data/24heart_small.h \
data/34heart_small.h \
data/allan.h \
data/arrows.h \
data/blood.h \
data/chargebar.h \
data/chargebarr.h \
data/drawing.h \
data/emptyheart_small.h \
data/fullheart_small.h \
data/goldhead.h \
data/head.h \
data/howie-scream.h \
data/idol.h \
data/indiana-low.h \
data/indiana.h \
data/indy-idol.h \
data/indy.h \
data/logo.h \
data/map01.h \
data/map01_hit.h \
data/patrocinador.h \
data/power.h \
data/special_whip.h \
data/wakeup_1.h \
data/wakeup_2.h \
data/wakeup_3.h \
data/whip_1.h \
data/whip_2.h \
data/whip_3.h \
data/whip_4.h \
data/whip_5.h \
data/wilhelm_scream.h \
data/zombie.h \
data/zombie_1.h \
data/zombie_2.h \
data/zombie_3.h

.PHONY: all clean depend

all: $(OUTPUT)

clean:
	rm -fv hardheaded hardheaded.exe $(OBJS) .depend gmon.out keys.ini HardHeaded.app/Contents/MacOS/HardHeaded
	make -C data clean

$(INCS):
	make -C data

delend: .depend

.depend: $(patsubst %.o,%.c,$(OBJS)) | $(INCS)
	$(CC) $(CFLAGS) -MM $^ > $@

ifneq ($(MAKECMDGOALS),clean)
-include .depend
endif

$(OUTPUT): $(OBJS) | $(INCS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@
	mkdir -p HardHeaded.app/Contents/MacOS/
	cp -fv $@ HardHeaded.app/Contents/MacOS/HardHeaded

