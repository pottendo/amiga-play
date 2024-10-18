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
CPROGRAMS = hello-world hello-pthread time-test scratch #anim #bobs-sprites
CPPPROGRAMS = mandel mandel13 #hello-cpp 
PROGRAMS = $(CPROGRAMS) $(CPPPROGRAMS)
COBJECTS = $(addsuffix .o,$(CPROGRAMS))
CPPOBJECTS = $(addsuffix .o,$(CPPPROGRAMS))

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

$(HDISKNAME): $(PROGRAMS) Startup-Sequence
	rm -f $(HDISKNAME)
	$(XDFTOOL) $(HDISKNAME) create size=10Mi
	make disk D=$@ VN=$(shell basename $@ .hdf)

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
anim: anim.o animtools.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm
	$(STRIP) $@

mandel.o: mandel.cpp mandelbrot.h
	$(CPLUSPLUS) $(CPPFLAGS) -c $< -o $@ -DPTHREADS 

mandel: mandel.o posix-clockfnpp.o #anim.o animtools.o
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^ -lpthread
	$(STRIP) $@

mandel13.o: mandel.cpp mandelbrot.h
	$(CPLUSPLUS) -g -Wall $(OPTIMIZE) -mcrt=nix13 -DKICK1 -c $< -o $@

mandel13: mandel13.o posix-clockfnpp.o anim.o animtools.o
	$(CPLUSPLUS) -N -mcrt=nix13 -o $@ $^ -lpthread
	$(STRIP) $@

hello-cpp: hello-cpp.o 
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@

%.o: %.cpp
	$(CPLUSPLUS) $(CPPFLAGS) -o $@ -c $^

clean:
	rm -f $(DISKNAME) $(HDISKNAME) $(PROGRAMS) *.o
	rm -f "$(FSUAEBASE)/Save States/a500hdkick13/`basename $(DISKNAME) .adf`.sdf" "$(FSUAEBASE)/Save States/a500hdkick13/`basename $(HDISKNAME) .hdf`.sdf" 
	rm -f "$(FSUAEBASE)/Save States/a500+hdkick3.2.1/`basename $(DISKNAME) .adf`.sdf" "$(FSUAEBASE)/Save States/a500+hdkick3.2.1/`basename $(HDISKNAME) .hdf`.sdf"

%.o: %.c
	$(CC) $(CFLAGS) -c  $<

#%.o: %.cpp
#	$(CPLUSPLUS) $(CPPFLAGS) -c $<
