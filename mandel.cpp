#ifdef __amiga__
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/io.h>
#include <inline/timer.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <graphics/sprite.h>
#define CLOCK_GETTIME
#else
#define CLOCK_GETTIME
#endif
#include <stdarg.h>
#ifdef PTHREADS
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#define NO_THREADS 16 // max 16 for Orangecart!
pthread_mutex_t logmutex;
void log_msg(const char *s, ...)
{
    char t[256];
    va_list args;

    //    pthread_mutex_lock(&logmutex);
    va_start(args, s);
    vsnprintf(t, 256, s, args);
    printf(t);
    //    pthread_mutex_unlock(&logmutex);
}
#else
// #define sleep(...)
#define NO_THREADS 1 // singlethreaded
#define log_msg printf
#endif

#include <cstring>
#include <math.h>
#include <vector>

#define PIXELW 2 // 2
#define MAX_ITER 32 * 2
#define IMG_W (320 * 2)      // 320
#define IMG_H (128 * 2 - 20) // 200
#define MTYPE double

#define CSIZE (IMG_W * IMG_H) / 8
#define PAL_SIZE 64 //(2 * PIXELW)

// set this to enable direct output on C64 gfx mem.
// #define C64
#define BUFSIZE (100)
#define MYSTRGADWIDTH (200)
#define MYSTRGADHEIGHT (8)

WORD strBorderData[] =
    {
    0,0, MYSTRGADWIDTH + 3,0, MYSTRGADWIDTH + 3,MYSTRGADHEIGHT + 3,
    0,MYSTRGADHEIGHT + 3, 0,0,
    };
struct Border strBorder =
    {
    -2,-2,1,0,JAM1,5,strBorderData,NULL,
    };
char strBuffer[BUFSIZE];
char strUndoBuffer[BUFSIZE];
struct StringInfo strInfo =
    {
    strBuffer,strUndoBuffer,0,BUFSIZE, /* compiler sets remaining fields to zero */
    };
struct Gadget strGad =
    {
    NULL, 20,20,MYSTRGADWIDTH,MYSTRGADHEIGHT,
    GFLG_GADGHCOMP, GACT_RELVERIFY | GACT_STRINGCENTER,
    GTYP_STRGADGET, &strBorder, NULL, NULL,0,&strInfo,0,NULL,
    };

// #define NO_LOG
#ifdef NO_LOG
#define log_msg(...)
#endif

#ifdef C64
#include "c64-lib.h"
#else
static char cv[CSIZE] = {};
// char *cv;
#endif

#ifdef PTHREAD_STACK_MIN
#define STACK_SIZE PTHREAD_STACK_MIN
#else
#define STACK_SIZE 1024
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
MTYPE xrat = 1.0;

typedef struct
{
    point_t lu;
    point_t rd;
} rec_t;
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
#endif
std::vector<rec_t> recs = {
    {{00, 00}, {IMG_W / 2, IMG_H / 2}},
    {{IMG_W / 4, IMG_H / 4}, {IMG_W / 4 + IMG_W / 4, IMG_H / 4 + IMG_H / 4}},
    {{IMG_W / 4, IMG_H / 4}, {IMG_W / 4 + IMG_W / 4, IMG_H / 4 + IMG_H / 4}},
};

#ifdef __amiga__
static struct Screen *myScreen;
static struct Window *myWindow;
static struct RastPort *rp;
static struct SimpleSprite sprite_ul = {0};
static struct SimpleSprite sprite_lr = {0};
static char title[24] = "Mandelbrot";
static struct NewScreen Screen1 = {
    0, 0, 320 * 2, 360, 6, /* Screen of 640 x 480 of depth 8 (2^8 = 256 colours)    */
    DETAILPEN, BLOCKPEN,
    EXTRA_HALFBRITE, /* see graphics/view.h for view modes */
    CUSTOMSCREEN,    /* Screen types */
    NULL,            /* Text attributes (use defaults) */
    (char *)title,
    NULL,
    NULL};

#define WINX (IMG_W / 1)
#define WINY (IMG_H / 1)
/* real boring sprite data */
UWORD __chip sprite_data_ul[] = {
    0, 0,           /* position control           */
    0xffff, 0x0000, /* image data line 1, color 1 */
    0xffff, 0x0000, /* image data line 2, color 1 */
    0xC000, 0x0000, /* image data line 3, color 2 */
    0xC000, 0x0000, /* image data line 4, color 2 */
    0xC000, 0x0000, /* image data line 5, transparent */
    0xC000, 0x0000, /* image data line 6, color 2 */
    0xC000, 0x0000, /* image data line 7, color 2 */
    0xC000, 0x0000, /* image data line 8, color 3 */
    0xC000, 0x0000, /* image data line 9, color 3 */
    0xC000, 0x0000, /* image data line 10, color 3 */
    0xC000, 0x0000, /* image data line 11, color 3 */
    0xC000, 0x0000, /* image data line 12, color 3 */
    0xC000, 0x0000, /* image data line 13, color 3 */
    0xC000, 0x0000, /* image data line 14, color 3 */
    0xC000, 0x0000, /* image data line 15, color 3 */
    0xC000, 0x0000, /* image data line 16, color 3 */
    0, 0            /* reserved, must init to 0 0 */
};

UWORD __chip sprite_data_lr[] = {
    0, 0,           /* position control           */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0x0003, 0x0000, /* image data line 1, color 1 */
    0xffff, 0x0000, /* image data line 1, color 1 */
    0xffff, 0x0000, /* image data line 2, color 1 */
    0, 0            /* reserved, must init to 0 0 */
};

void sprite_setup(struct Screen *myScreen)
{
    WORD sprnum;
    SHORT color_reg;
    struct ViewPort *viewport;
    viewport = &myScreen->ViewPort;

    sprnum = GetSprite(&sprite_ul, 2);
    // printf("sprite ul num = %d\n", sprnum);
    /* Calculate the correct base color register number, */
    /* set up the color registers.                       */
    color_reg = 16 + ((sprnum & 0x06) << 1);
    // printf("color_reg=%d\n", color_reg);
    SetRGB4(viewport, color_reg + 1, 0xf, 0x0, 0x0);

    sprite_ul.x = 0;       /* initialize position and size info    */
    sprite_ul.y = 0;       /* to match that shown in sprite_data   */
    sprite_ul.height = 16; /* so system knows layout of data later */

    /* install sprite data and move sprite to start position. */
    ChangeSprite(NULL, &sprite_ul, sprite_data_ul);
    MoveSprite(NULL, &sprite_ul, -1, 20);

    sprnum = GetSprite(&sprite_lr, 3);
    // printf("sprite lr num = %d\n", sprnum);
    /* Calculate the correct base color register number, */
    /* set up the color registers.                       */
    color_reg = 16 + ((sprnum & 0x06) << 1);
    // printf("color_reg=%d\n", color_reg);
    SetRGB4(viewport, color_reg + 1, 0xf, 0x0, 0x0);

    sprite_lr.x = 0;       /* initialize position and size info    */
    sprite_lr.y = 0;       /* to match that shown in sprite_data   */
    sprite_lr.height = 16; /* so system knows layout of data later */

    /* install sprite data and move sprite to start position. */
    ChangeSprite(NULL, &sprite_lr, sprite_data_lr);
    MoveSprite(NULL, &sprite_lr, WINX / 2 - 16, WINY + 4);
}

void setup_screen(void)
{
    myScreen = OpenScreen(&Screen1); /* & (ampersand) means address of */
    ScreenToFront(myScreen);
    ShowTitle(myScreen, TRUE);
    MakeScreen(myScreen);
    sprite_setup(myScreen);
    struct NewWindow winlayout = {
        0 * WINX / 4, 0 * WINY / 4 + 10,
        WINX, WINY + 10,
        0, 1,
        IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY,
        WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE,
        NULL, NULL,
        (char *)title,
        myScreen, NULL,
        0, 0,
        WINX, WINY,
        CUSTOMSCREEN};
    myWindow = OpenWindow(&winlayout);
    rp = myWindow->RPort;
}

int amiga_setpixel(void *not_used, int x, int y, int col)
{
    //    log_msg("%s: %dx%d->%d\n", __FUNCTION__, x, y, col);
    //pthread_mutex_lock(&logmutex);
    SetAPen(rp, col);
    WritePixel(rp, x, y + 10);
    //    Move(rp, x, y+10);
    //    Draw(rp, WINX, y + 10);
    //pthread_mutex_unlock(&logmutex);
    struct Message *pMsg;
    int ret = 0;
    if ((pMsg = GetMsg(myWindow->UserPort)) != NULL)
    {
        struct IntuiMessage *pIMsg = (struct IntuiMessage *)pMsg;
        switch (pIMsg->Class)
        {
        case IDCMP_MOUSEBUTTONS:
            ret = 1;
            ReplyMsg(pMsg);
            while (1) // loop until button up is seen
            {
                pMsg = GetMsg(myWindow->UserPort);
                if (pMsg)
                {
                    pIMsg = (struct IntuiMessage *)pMsg;
                    if (pIMsg->Code == SELECTUP)
                        goto out;
                    ReplyMsg(pMsg);
                }
            }
            goto out;
            break;
        case IDCMP_CLOSEWINDOW:
            ret = 1;
            break;
        default:
            log_msg("%s: class = %ld\n", __FUNCTION__, pIMsg->Class);
            break;
        }
        ReplyMsg(pMsg);
    }
out:
    return ret;
}

void amiga_zoom(mandel<MTYPE> *m)
{
    uint16_t stx = 0, sty = 10;
    bool closewin = FALSE;
    while (closewin == FALSE)
    {
        WaitPort(myWindow->UserPort);

        struct Message *pMsg;
        while ((pMsg = GetMsg(myWindow->UserPort)) != NULL)
        {
            struct IntuiMessage *pIMsg = (struct IntuiMessage *)pMsg;

            switch (pIMsg->Class)
            {
            case IDCMP_CLOSEWINDOW:
                closewin = TRUE;
                break;
            case IDCMP_MOUSEMOVE:
                if (pIMsg->MouseX >= WINX)
                    break;
                if (pIMsg->MouseY >= WINY + 10)
                    break;
                if (pIMsg->MouseY < 10)
                    break;
                MoveSprite(NULL, &sprite_lr, pIMsg->MouseX - 16, pIMsg->MouseY - 4);
#if 0		
		SetDrMd(rp, COMPLEMENT);
		SetAPen(rp, 0);
		RectFill(rp, stx, sty, pIMsg->MouseX, pIMsg->MouseY);
		SetDrMd(rp, JAM1);
#endif
                if ((stx < pIMsg->MouseX) &&
                    (sty < pIMsg->MouseY))
                    break;
                if (stx >= pIMsg->MouseX)
                    stx = pIMsg->MouseX;
                if (sty >= pIMsg->MouseY)
                    sty = pIMsg->MouseY;
                MoveSprite(NULL, &sprite_ul, stx - 1, sty + 10);
                break;
            case IDCMP_MOUSEBUTTONS:
                if (pIMsg->Code == SELECTDOWN)
                {
                    stx = pIMsg->MouseX;
                    sty = pIMsg->MouseY;
                    // log_msg("mouse select start: (%d,%d)\n", stx, sty);
                    ReportMouse(TRUE, myWindow);
                    MoveSprite(NULL, &sprite_ul, pIMsg->MouseX - 1, pIMsg->MouseY + 10);
                }
                if (pIMsg->Code == SELECTUP)
                {
                    ReportMouse(FALSE, myWindow);
                    MoveSprite(NULL, &sprite_ul, -1, 20);
                    MoveSprite(NULL, &sprite_lr, WINX / 2 - 16, WINY + 4);
                    if ((stx == pIMsg->MouseX) ||
                        (sty == pIMsg->MouseY) ||
                        (sty < 10) ||
                        (pIMsg->MouseX > WINX) ||
                        (pIMsg->MouseY > WINY))
                    {
                        // log_msg("stx=%d, sty=%d, Mx=%d, My=%d\n", stx, sty, pIMsg->MouseX, pIMsg->MouseY);
                        DisplayBeep(myScreen);
                        break;
                    }
                    point_t lu{stx, (uint16_t)(sty - 10)}, rd{(uint16_t)pIMsg->MouseX, (uint16_t)(pIMsg->MouseY - 10)};
                    m->select_start(lu);
                    m->select_end(rd);
                    DisplayBeep(myScreen);
                    ReportMouse(FALSE, myWindow);
                }
                break;
            case IDCMP_RAWKEY:
                log_msg("%s: RAWKEY event\n", __FUNCTION__);
                break;
            }
            ReplyMsg(pMsg);
        }
    }
    CloseWindow(myWindow);
    if (myScreen)
        CloseScreen(myScreen); /* Close screen using myScreen pointer */
}

#else
#define setup_screen()
#endif // __amiga__

int main(void)
{
    pthread_mutex_init(&logmutex, NULL);
    log_msg("Welcome mandelbrot...\n");
#ifndef CONFIG_BOARD_ORANGECART
    // stacks = (char *) alloca(STACK_SIZE * NO_THREADS); //new char[STACK_SIZE * NO_THREADS]();
    stacks = new char[STACK_SIZE * NO_THREADS]();
    log_msg("%s: stack_size per thread = %d, no threads=%d\n", __FUNCTION__, STACK_SIZE, NO_THREADS);
#endif

#ifdef C64
    c64 c64;
    // std::cout << "C64 memory @0x" << std::hex << int(c64.get_mem()) << std::dec << '\n';
    char *cv = (char *)&c64.get_mem()[0x4000];
    c64.screencols(VIC::BLACK, VIC::BLACK);
    c64.gfx(VICBank1, VICModeGfxMC, 15);
    // xrat = 16.0 / 9.0;
#endif
#ifndef __amiga__
    int col1, col2, col3;
    col1 = 0xb;
    col2 = 0xc;
    col3 = 14; // VIC::LIGHT_BLUE;
#endif
    setup_screen();
    for (int i = 0; i < 1; i++)
    {
#ifdef C64
        memset(&cv[0x3c00], (col1 << 4) | col2, 1000);
        memset(&c64.get_mem()[0xd800], col3, 1000);
#endif
        mandel<MTYPE> *m = new mandel<MTYPE>{cv, stacks, -1.5, -1.0, 0.5, 1.0, IMG_W / PIXELW, IMG_H, xrat};
#ifdef __amiga__
        amiga_zoom(m);
#else
        for (size_t i = 0; i < recs.size(); i++)
        {
            auto it = &recs[i];
            log_msg("%d/%d, zooming into [%d,%d]x[%d,%d]...stacks=%p\n", i, recs.size(), it->lu.x, it->lu.y, it->rd.x, it->rd.y, m->get_stacks());
            m->select_start(it->lu);
            m->select_end(it->rd);
        }
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
