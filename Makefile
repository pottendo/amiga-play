PREFIX = /opt/amiga13/bin
CC = $(PREFIX)/m68k-amigaos-gcc
CPLUSPLUS = $(PREFIX)/m68k-amigaos-g++
STRIP = $(PREFIX)/m68k-amigaos-strip
XDFTOOL = xdftool
DISKNAME = hack.adf
HDISKNAME = playhd.hdf
CFLAGS = -g -Wall $(OPTIMIZE) $(KICK)
CPPFLAGS = -g -Wall $(OPTIMIZE) $(KICK)
OPTIMIZE = -O3
LDFLAGS = $(KICK) -N
CPROGRAMS = hello-world hello-pthread time-test #bobs-sprites
CPPPROGRAMS = hello-cpp mandel mandel13
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
	$(XDFTOOL) $(D) format "play"
	$(XDFTOOL) $(D) boot install boot1x
	$(XDFTOOL) $(D) makedir s
	$(XDFTOOL) $(D) write Startup-Sequence s
	$(XDFTOOL) $(D) write libs
	$(XDFTOOL) $(D) delete s/Startup-Sequence
	$(XDFTOOL) $(D) write Startup-Sequence s
	$(XDFTOOL) $(D) delete pottendo all || exit 0
	$(XDFTOOL) $(D) makedir pottendo
	$(XDFTOOL) $(D) write c
	for f in $(PROGRAMS) ; do \
		$(XDFTOOL) $(D) write $$f pottendo ; /bin/true ;\
	done

$(DISKNAME): $(PROGRAMS) Startup-Sequence
	rm -f $(DISKNAME)
	$(XDFTOOL) $(DISKNAME) create
	make disk D=$@

$(HDISKNAME): $(PROGRAMS) Startup-Sequence
	rm -f $(HDISKNAME)
	$(XDFTOOL) $(HDISKNAME) create size=10Mi
	make disk D=$@

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
bobs-sprites: bobs-sprites.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm
	$(STRIP) $@

mandel.o: mandel.cpp mandelbrot.h
	$(CPLUSPLUS) $(CPPFLAGS) -DPTHREADS -c $< -o $@

mandel: mandel.o posix-clockfnpp.o
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^ -lpthread
	$(STRIP) $@

mandel13.o: mandel.cpp mandelbrot.h
	$(CPLUSPLUS) -g -Wall $(OPTIMIZE) -mcrt=nix13 -DKICK1 -c $< -o $@

mandel13: mandel13.o posix-clockfnpp.o
	$(CPLUSPLUS) -N -mcrt=nix13 -o $@ $^ -lpthread
	$(STRIP) $@

hello-cpp: hello-cpp.o 
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@

%.o: %.cpp
	$(CPLUSPLUS) $(CPPFLAGS) -o $@ -c $^

clean:
	rm -f $(DISKNAME) $(HDISKNAME) $(PROGRAMS) *.o

%.o: %.c
	$(CC) $(CFLAGS) -c  $<

#%.o: %.cpp
#	$(CPLUSPLUS) $(CPPFLAGS) -c $<
