PREFIX = /opt/amiga13/bin
CC = $(PREFIX)/m68k-amigaos-gcc
CPLUSPLUS = $(PREFIX)/m68k-amigaos-g++
STRIP = $(PREFIX)/m68k-amigaos-strip
XDFTOOL = xdftool
FSUAEBASE=$(shell echo ${HOME})/Documents/FS-UAE
DISKNAME = playfd.adf
HDISKNAME = playhd.hdf
CFLAGS = -g -Wall $(OPTIMIZE) $(KICK)
CPPFLAGS = -g -Wall $(OPTIMIZE) $(KICK)
OPTIMIZE = -O3
LDFLAGS = $(KICK) -N
CPROGRAMS = anims anims13 #bobs-sprites
CPPPROGRAMS = par-play par-play13 par-test par-test13 #mandel mandel13 #hello-cpp 
PROGRAMS = $(CPROGRAMS) $(CPPPROGRAMS)
COBJECTS = $(addsuffix .o,$(CPROGRAMS))
CPPOBJECTS = $(addsuffix .o,$(CPPPROGRAMS))
MSOURCE = ../../pottendo-mandel
SUPW = -Wno-unused-but-set-variable -Wno-parentheses -Wno-int-conversion -Wno-unused-variable \
	   -Wno-implicit-function-declaration -Wno-maybe-uninitialized -Wno-main

ifeq ($(KICK1),)
	KICK= -mcrt=nix20
else
	KICK=-mcrt=nix13
	CFLAGS += -DKICK1
	CPPFLAGS += -DKICK1
endif

all: $(DISKNAME) $(HDISKNAME)

disk: 
	$(XDFTOOL) $(D) format "$(VN)"
	$(XDFTOOL) $(D) boot install boot1x
	$(XDFTOOL) $(D) write libs
	$(XDFTOOL) $(D) makedir s
	$(XDFTOOL) $(D) write Startup-Sequence s
	$(XDFTOOL) $(D) write c
	$(XDFTOOL) $(D) makedir pottendo
	for f in $(PROGRAMS) ; do \
		$(XDFTOOL) $(D) write $$f pottendo ;\
	done

$(DISKNAME): $(PROGRAMS) Startup-Sequence
	rm -f $(DISKNAME)
	$(XDFTOOL) $(DISKNAME) create
	make disk D=$@ VN=$(shell basename $@ .adf)
	$(XDFTOOL) $@ write devs13 devs

$(HDISKNAME): $(PROGRAMS) Startup-Sequence
	rm -f $(HDISKNAME)
	$(XDFTOOL) $(HDISKNAME) create size=10Mi
	make disk D=$@ VN=$(shell basename $@ .hdf)
	$(XDFTOOL) $@ write devs31 devs

#$(CPROGRAMS): $(COBJECTS)
hello-world: hello-world.o posix-clockfn.o
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@
hello-pthread: hello-pthread.o
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread
	$(STRIP) $@
gui-hack: gui-hack.o mandellib.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm
	$(STRIP) $@

anims.o: anim.c
	$(CC) -Wall $(OPTIMIZE) $(CFLAGS) $(SUPW) -DANIM_STANDALONE -c $< -o $@

anims: anims.o animtools.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm
	$(STRIP) $@

mandel.o: $(MSOURCE)/mandel.cpp $(MSOURCE)/mandelbrot.h $(MSOURCE)/mandel-arch.h
	$(CPLUSPLUS) -I$(MSOURCE) $(CPPFLAGS) -c $< -o $@ -DPTHREADS 

mandel-amiga.o: $(MSOURCE)/mandel-amiga.cpp $(MSOURCE)/mandel-arch.h $(MSOURCE)/mandelbrot.h
	$(CPLUSPLUS) -I$(MSOURCE) $(CPPFLAGS) -c $< -o $@ -DPTHREADS 

anim.o: anim.c
	$(CC) -Wall $(OPTIMIZE) $(CFLAGS) -I$(MSOURCE) $(SUPW) -DSBMDEPTH=SCRDEPTH -DRBMDEPTH=SCRDEPTH -c $< -o $@

mandel: mandel.o posix-clockfnpp.o anim.o animtools.o mandel-amiga.o
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^ -lpthread
	$(STRIP) $@

mandel13.o: $(MSOURCE)/mandel.cpp $(MSOURCE)/mandelbrot.h $(MSOURCE)/mandel-arch.h
	$(CPLUSPLUS) -I$(MSOURCE) -Wall $(OPTIMIZE) -mcrt=nix13 -DKICK1 -c $< -o $@

mandel-amiga13.o: $(MSOURCE)/mandel-amiga.cpp $(MSOURCE)/mandel-arch.h
	$(CPLUSPLUS) -I$(MSOURCE) -Wall $(OPTIMIZE) -mcrt=nix13 -DKICK1 -c $< -o $@

anim13.o: anim.c
	$(CC) -Wall $(OPTIMIZE) -I$(MSOURCE) $(SUPW) -DSBMDEPTH=SCRDEPTH -DRBMDEPTH=SCRDEPTH -mcrt=nix13 -DKICK1 -c $< -o $@

animtools13.o: animtools.c
	$(CC) -Wall $(OPTIMIZE) $(SUPW) -mcrt=nix13 -DKICK1 -c $< -o $@

posix-clockfncpp13.o: posix-clockfnpp.cpp posix-clockfn.c
	$(CPLUSPLUS) -g -Wall  $(OPTIMIZE) -mcrt=nix13 -DKICK1 -c $< -o $@

mandel13: mandel13.o posix-clockfncpp13.o anim13.o animtools13.o mandel-amiga13.o
	$(CPLUSPLUS) -N -mcrt=nix13 -o $@ $^
	$(STRIP) $@

anims13.o: anim.c
	$(CC) -Wall $(OPTIMIZE) $(SUPW) -DANIM_STANDALONE -mcrt=nix13 -DKICK1 -c $< -o $@

anims13: anims13.o animtools13.o
	$(CPLUSPLUS) -N -mcrt=nix13 -o $@ $^
	$(STRIP) $@

hello-cpp: hello-cpp.o 
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@

par-play: par-play.o 
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@	

par-play13.o: par-play.cpp
	$(CPLUSPLUS) -Wall $(OPTIMIZE) -mcrt=nix13 -DKICK1 -c $< -o $@

par-play13: par-play13.o 
	$(CPLUSPLUS) -N -mcrt=nix13  -o $@ $^
	$(STRIP) $@	

par-test: par-test.o 
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@	

par-test13.o: par-test.cpp
	$(CPLUSPLUS) -Wall $(OPTIMIZE) -mcrt=nix13 -DKICK1 -c $< -o $@

par-test13: par-test13.o 
	$(CPLUSPLUS) -N -mcrt=nix13  -o $@ $^
	$(STRIP) $@	
%.o: %.cpp
	$(CPLUSPLUS) $(CPPFLAGS) -o $@ -c $^

clean:
	rm -f $(DISKNAME) $(HDISKNAME) $(PROGRAMS) *.o
	rm -f "$(FSUAEBASE)/Save States/a500hdkick13/`basename $(DISKNAME) .adf`.sdf" "$(FSUAEBASE)/Save States/a500hdkick13/`basename $(HDISKNAME) .hdf`.sdf" 
	rm -f "$(FSUAEBASE)/Save States/a500+hdkick3.2.1/`basename $(DISKNAME) .adf`.sdf" "$(FSUAEBASE)/Save States/a500+hdkick3.2.1/`basename $(HDISKNAME) .hdf`.sdf"

%.o: %.c
	$(CC) $(CFLAGS) $(SUPW) -c  $<

#%.o: %.cpp
#	$(CPLUSPLUS) $(CPPFLAGS) -c $<
