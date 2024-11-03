/* definition section for globals */
#define SCRDEPTH 4  // or 6 for 64cols lesser resolution
#define MTYPE double

#define CSIZE (IMG_W * IMG_H) / 8
#define PAL_SIZE (1L << SCRDEPTH)
#define PIXELW 1 // 2
#define MAX_ITER 64

void log_msg(const char *s, ...);

#ifdef PTHREADS
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#define NO_THREADS 16 // max 16 for Orangecart!
#ifdef PTHREAD_STACK_MIN
#define STACK_SIZE PTHREAD_STACK_MIN
#else
#define STACK_SIZE 1024
#endif
extern pthread_mutex_t logmutex;
#else
#define NO_THREADS 1 // singlethreaded
#define log_msg printf
#define STACK_SIZE 1024
#endif

#ifdef __amiga__
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/io.h>
#include <inline/timer.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <graphics/sprite.h>
#include <exec/memory.h>
#include <devices/inputevent.h>
#include <clib/console_protos.h>
#include <vector>
#if (SCRDEPTH > 6)
#error "pixeldepth too large, must be < 6"
#endif
#if (SCRDEPTH <= 4)
#define HALF 1
//#define HALF 2
#define SCRMODE (HIRES|LACE)
//#define SCRMODE EXTRA_HALFBRITE
#define SCMOUSE 2
//#define SCMOUSE 1
#else
#define HALF 2
#define SCRMODE EXTRA_HALFBRITE
#define SCMOUSE 1
#endif

#define IMG_W (640 / HALF)      // 320
#define IMG_H (512 / HALF - 20) // 200
#define WINX (IMG_W / 1)
#define WINY (IMG_H / 1)

#undef MAX_ITER
#define MAX_ITER iter
extern int iter;
#include "mandelbrot.h"
void amiga_setup_screen(void);
void amiga_zoom_ui(mandel<MTYPE> *m);

#define setup_screen amiga_setup_screen
#define zoom_ui amiga_zoom_ui

#endif  /* __amiga__ */

#ifdef LUCKFOX
#define IMG_W img_w
#define IMG_H img_h

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

#include <stdint.h>

extern uint16_t *tft_canvas;		// must not be static?!

void luckfox_setpx(void *canvas, int x, int y, uint16_t c);
void init_luckfox(void);
void luckfox_palette(uint16_t *p);
void luckfox_play(void);
void luckfox_rect(int x1, int y1, int x2, int y2, uint16_t c);

#else
#define init_luckfox(...)
#define luckfox_setpx(...)
#define luckfox_play(...)

#endif /* LUCKFOX */

