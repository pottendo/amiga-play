/* definition section for globals */
#define MTYPE int  //double
#define INTMATH    // goes along with int above, on Intels or other fast FPUs, double/float can be faster
#define MAX_ITER_INIT 64

#ifdef INTMATH
#define INTSCALE 1024
#define INTIFY(a) ((a) * INTSCALE)
#define INTIFY2(a) ((a) * INTSCALE * INTSCALE)
#define DEINTIFY(a) ((a) / INTSCALE)
#else
#define INTIFY(a) (a)
#define INTIFY2(a) (a)
#define DEINTIFY(a) (a)
#endif

void log_msg(const char *s, ...);
// #define NO_LOG
#ifdef NO_LOG
#define log_msg(...)
#endif

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
#define MAX_ITER iter   // relict

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

#define SCRDEPTH 6  // or 6 for 64cols lesser resolution
#define PAL_SIZE (1L << SCRDEPTH)
#define PIXELW 1

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
#define CSIZE (IMG_W * IMG_H) / 8
#define WINX (IMG_W / 1)
#define WINY (IMG_H / 1)

extern int iter;
#include "mandelbrot.h"
void amiga_setup_screen(void);
void amiga_zoom_ui(mandel<MTYPE> *m);

#define setup_screen amiga_setup_screen
#define zoom_ui amiga_zoom_ui

#else  /* __amiga__ */

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

#define zoom_ui(...) luckfox_play()
#define setup_screen init_luckfox
#define canvas_setpx luckfox_setpx

#else /* else other architectures*/

// non-specific architectures 
#define IMG_W 200
#define IMG_H 100
#define SCRDEPTH 2
#define CSIZE (IMG_W * IMG_H) / 8
#define PAL_SIZE (1L << SCRDEPTH)
#define PIXELW 2 // 2

#define setup_screen(...)
#define canvas_setpx canvas_setpx_
#define zoom_ui(...)

#endif /* LUCKFOX */
#endif /* __amiga__ */
