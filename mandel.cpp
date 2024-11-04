#include <stdarg.h>
#include <cstring>
#include <math.h>
#include <vector>

#include "mandel-arch.h"

#ifdef PTHREADS
pthread_mutex_t logmutex;
pthread_mutex_t canvas_sem;
void log_msg(const char *s, ...)
{
    char t[256];
    va_list args;
    pthread_mutex_lock(&logmutex);
    va_start(args, s);
    vsnprintf(t, 256, s, args);
    printf(t);
    pthread_mutex_unlock(&logmutex);
}
#endif  /* PTHREADS */

// globals
int img_w, img_h;   // used by luckfox
int iter = MAX_ITER_INIT;  // used by Amiga
MTYPE xrat = 1.0;

// set this to enable direct output on C64 gfx mem.
// #define C64
#ifdef C64
#include "c64-lib.h"
#else
char *cv;
#endif

#ifdef CONFIG_BOARD_ORANGECART
#if (NO_THREADS > 16)
#error "too many threads for Orangencart's STACK_SIZE"
#endif
static char *stacks = (char *)0x10000000; // fast SRAM on Orangecart, only 16k! so NO_THREADS <= 16
#else
static char *stacks; // stacks[STACK_SIZE * NO_THREADS];
#endif

#include "mandelbrot.h"

typedef struct
{
    point_t lu;
    point_t rd;
} rec_t;

int main(void)
{
    pthread_mutex_init(&logmutex, NULL);
    log_msg("Welcome mandelbrot...\n");
#ifndef CONFIG_BOARD_ORANGECART
    // stacks = (char *) alloca(STACK_SIZE * NO_THREADS); //new char[STACK_SIZE * NO_THREADS]();
    stacks = new char[STACK_SIZE * NO_THREADS]();
    log_msg("%s: stack_size per thread = %d, no threads=%d, iter = %d, palette = %ld\n", __FUNCTION__, STACK_SIZE, NO_THREADS, iter, PAL_SIZE);
#endif

#ifdef C64
    c64 c64;
    // std::cout << "C64 memory @0x" << std::hex << int(c64.get_mem()) << std::dec << '\n';
    char *cv = (char *)&c64.get_mem()[0x4000];
    c64.screencols(VIC::BLACK, VIC::BLACK);
    c64.gfx(VICBank1, VICModeGfxMC, 15);
    // xrat = 16.0 / 9.0;
    int col1, col2, col3;
    col1 = 0xb;
    col2 = 0xc;
    col3 = 14; // VIC::LIGHT_BLUE;
#endif
    cv = new char[CSIZE]();
    setup_screen();
#if 0
std::vector<rec_t> recs = { 
        {{00, 00},{80,100}}, 
        {{80, 100},{159,199}}, 
        {{00, 50},{40,100}},        
        {{80, 110}, {120, 160}},
        {{60,75}, {100, 125}},
        {{60,110}, {100, 160}},
        {{60,75}, {100, 125}},
        {{60,75}, {100, 125}},
        {{40,50}, {80, 100}},
        {{120,75}, {159, 125}},
    };
#else
std::vector<rec_t> recs = {
    {{00, IMG_H / 4}, {IMG_W / 2, IMG_H / 4 * 3}},
    {{00, IMG_H / 4}, {IMG_W / 2, IMG_H / 4 * 3}},
    {{IMG_W / 4, IMG_H / 4}, {IMG_W / 4 + 200, IMG_H / 4 * 3}},
};
#endif

    for (int i = 0; i < 1; i++)
    {
#ifdef C64
        memset(&cv[0x3c00], (col1 << 4) | col2, 1000);
        memset(&c64.get_mem()[0xd800], col3, 1000);
#endif
        mandel<MTYPE> *m = new mandel<MTYPE>{cv, stacks, 
                                            static_cast<MTYPE>(-1.5), static_cast<MTYPE>(-1.0), 
                                            static_cast<MTYPE>(0.5), static_cast<MTYPE>(1.0), 
                                            IMG_W / PIXELW, IMG_H, xrat};
        zoom_ui(m);
        return 0;
        for (size_t i = 0; i < recs.size(); i++)
        {
            auto it = &recs[i];
            log_msg("%ld/%ld, zooming into [%d,%d]x[%d,%d]...stacks=%p\n", i, recs.size(), it->lu.x, it->lu.y, it->rd.x, it->rd.y, m->get_stacks());
            m->select_start(it->lu);
            m->select_end(it->rd);
        }
#ifdef C64        
        col1++; col2++; col3++;
        col1 %= 0xf;
        if (col1 == 0)
            col1++;
        col2 %= 0xf;
        if (col2 == 0)
            col2++;
        col3 %= 0xf;
        if (col3 == 0)
            col3++;
#endif        
        delete m;
    }
#ifdef __ZEPHYR__
    while (1)
    {
        // std::cout << "system halted.\n";
        sleep(10);
    }
#endif
    return 0;
}
