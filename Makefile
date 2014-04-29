
SDL2_FLAGS = `sdl2-config --cflags` `sdl2-config --libs`
SODNADIR=src/sodna-0.2.0
CFLAGS=-Isrc/brogue -Isrc/platform -Wall -Wno-parentheses ${DEFINES}
RELEASENAME=brogue-1.7.2
LASTTARGET := $(shell ./brogue --target)
CC ?= gcc

ifeq (${LASTTARGET},both)
all : both
else ifeq (${LASTTARGET},curses)
all : curses
else ifeq (${LASTTARGET},sodna)
all : sodna
else
all : both
endif

%.o : %.c Makefile src/brogue/Rogue.h src/brogue/IncludeGlobals.h
	$(CC) $(CFLAGS) -g -o $@ -c $< 

BROGUEFILES=src/brogue/Architect.o \
	src/brogue/Combat.o \
	src/brogue/Dijkstra.o \
	src/brogue/Globals.o \
	src/brogue/IO.o \
	src/brogue/Items.o \
	src/brogue/Light.o \
	src/brogue/Monsters.o \
	src/brogue/Buttons.o \
	src/brogue/Movement.o \
	src/brogue/Recordings.o \
	src/brogue/RogueMain.o \
	src/brogue/Random.o \
	src/brogue/MainMenu.o \
	src/brogue/Grid.o \
	src/brogue/Time.o \
	src/platform/main.o \
	src/platform/platformdependent.o \
	src/platform/curses-platform.o \
	src/platform/sodna-platform.o \
	src/platform/sodna_util.o \
	src/platform/term.o

CURSES_DEF = -DBROGUE_CURSES
CURSES_LIB = -lncurses -lm

SODNA_DEF = -DBROGUE_SODNA -I${SODNADIR}/include
SODNA_DEP = ${SODNADIR}
SODNA_LIB = -L. -L${SODNADIR} ${SDL2_FLAGS} -lsodna_sdl2 -lm -Wl,-rpath,.

curses : DEFINES = ${CURSES_DEF}
curses : LIBRARIES = ${CURSES_LIB}

sodna : DEPENDENCIES += ${SODNA_DEP}
sodna : DEFINES += ${SODNA_DEF}
sodna : LIBRARIES += ${SODNA_LIB}

both : DEPENDENCIES += ${SODNA_DEP}
both : DEFINES += ${CURSES_DEF} ${SODNA_DEF}
both : LIBRARIES += ${CURSES_LIB} ${SODNA_LIB}

ifeq (${LASTTARGET},both)
both : bin/brogue
curses : clean bin/brogue
sodna : clean bin/brogue
else ifeq (${LASTTARGET},curses)
curses : bin/brogue
both : clean bin/brogue
sodna : clean bin/brogue
else ifeq (${LASTTARGET},sodna)
curses : clean bin/brogue
both : clean bin/brogue
sodna : bin/brogue
else
both : bin/brogue
curses : bin/brogue
sodna : bin/brogue
endif

.PHONY : clean both curses sodna tar

bin/brogue : ${DEPENDENCIES} ${BROGUEFILES}
	$(CC) -O2 -march=i586 -o bin/brogue ${BROGUEFILES} ${LIBRARIES} -Wl,-rpath,.

clean :
	rm -f src/brogue/*.o src/platform/*.o bin/brogue

${SODNADIR} :
	src/get-sodna.sh

tar : both
	rm -f ${RELEASENAME}.tar.gz
	tar --transform 's,^,${RELEASENAME}/,' -czf ${RELEASENAME}.tar.gz \
	Makefile \
	brogue \
	$(wildcard *.sh) \
	$(wildcard *.rtf) \
	readme \
	$(wildcard *.txt) \
	bin/brogue \
	bin/keymap \
	bin/icon.bmp \
	bin/brogue-icon.png \
	$(wildcard bin/fonts/*.png) \
	$(wildcard bin/*.so) \
	$(wildcard src/*.sh) \
	$(wildcard src/brogue/*.c) \
	$(wildcard src/brogue/*.h) \
	$(wildcard src/platform/*.c) \
	$(wildcard src/platform/*.h)

