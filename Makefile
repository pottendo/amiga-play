PREFIX = /opt/amiga13/bin
CC = $(PREFIX)/m68k-amigaos-gcc
CPLUSPLUS = $(PREFIX)/m68k-amigaos-g++
STRIP = $(PREFIX)/m68k-amigaos-strip
XDFTOOL = xdftool
DISKNAME = hack.adf
HDISKNAME = playhd.hdf
CFLAGS = -g -Wall $(OPTIMIZE) $(KICK)
CPPFLAGS = -g -Wall $(OPTIMIZE) $(KICK) -DPTHREADS 
#OPTIMIZE = -O3
LDFLAGS = $(KICK) -N
CPROGRAMS = gui-hack hello-world hello-pthread
CPPPROGRAMS = hello-cpp mandel
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
hello-world: hello-world.o
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@
hello-pthread: hello-pthread.o
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread
	$(STRIP) $@
gui-hack: gui-hack.o mandellib.o
	$(CC) $(LDFLAGS) -o $@ $^ -lm
	$(STRIP) $@

mandel.o: mandel.cpp mandelbrot.h
	$(CPLUSPLUS) $(CPPFLAGS) -c $@ $<

mandel: mandel.o
	$(CPLUSPLUS) $(LDFLAGS) -o $@ $^ -lpthread
	$(STRIP) $@

%: %.cpp
	$(CPLUSPLUS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^
	$(STRIP) $@

clean:
	rm -f $(DISKNAME) $(HDISKNAME) $(PROGRAMS) *.o

%.o: %.c
	$(CC) $(CFLAGS) -c  $<

#%.o: %.cpp
#	$(CPLUSPLUS) $(CPPFLAGS) -c $<
