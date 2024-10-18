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
extern "C"
{
    void run_setupAnimation(struct Window *w);
    void run_stepAnimation(void);
    struct GelsInfo *run_setupDisplay(struct Window *win,
                                      SHORT dbufing,
                                      struct BitMap **myBitMaps);
}

#define CLOCK_GETTIME
#else
#define CLOCK_GETTIME
#endif
#include <stdarg.h>
#ifdef PTHREADS
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#define NO_THREADS 4 // max 16 for Orangecart!
pthread_mutex_t logmutex;
pthread_mutex_t anim_ctrl;

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
#else
// #define sleep(...)
#define NO_THREADS 1 // singlethreaded
#define log_msg printf
#endif

#include <cstring>
#include <math.h>
#include <vector>

#define SCRDEPTH 4  // or 6 for 64cols lesser resolution

#if (SCRDEPTH <= 4)
//#define HALF 1
#define HALF 2
//#define SCRMODE (HIRES|LACE)
#define SCRMODE EXTRA_HALFBRITE
//#define SCMOUSE 2
#define SCMOUSE 1
#else
#define HALF 2
#define SCRMODE EXTRA_HALFBRITE
#define SCMOUSE 1
#endif
#define PIXELW 1 // 2
#define MAX_ITER iter
#define IMG_W (640 / HALF)      // 320
#define IMG_H (512 / HALF - 20) // 200
#define MTYPE double

#define CSIZE (IMG_W * IMG_H) / 8
#define PAL_SIZE (1L << SCRDEPTH)

// set this to enable direct output on C64 gfx mem.
// #define C64

int iter = 32 * 2;

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
//#define STACK_SIZE 1024
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
#define WINX (IMG_W / 1)
#define WINY (IMG_H / 1)
struct Library *ConsoleDevice;
struct IOStdReq ioreq;
static struct Screen *myScreen;
static struct Window *myWindow;
static struct RastPort *rp;
static struct SimpleSprite sprite_ul = {0};
static struct SimpleSprite sprite_lr = {0};
static char title[24] = "Mandelbrot";
static bool animation = true;
#if 1
static struct NewScreen Screen1 = {
    0, 0, IMG_W, IMG_H + 20, SCRDEPTH, /* Screen of 640 x 480 of depth 8 (2^8 = 256 colours)    */
    DETAILPEN, BLOCKPEN,
    SCRMODE, /* see graphics/view.h for view modes */
    CUSTOMSCREEN | SCREENQUIET,    /* Screen types */
    NULL,            /* Text attributes (use defaults) */
    (char *)title,
    NULL,
    NULL};
#else
static struct NewScreen Screen1 = {
    0, 0, IMG_W, IMG_H + 20, 4, /* Screen of 640 x 480 of depth 8 (2^8 = 256 colours)    */
    DETAILPEN, BLOCKPEN,
    LACE | HIRES, /* see graphics/view.h for view modes */
    CUSTOMSCREEN| SCREENQUIET,    /* Screen types */
    NULL,            /* Text attributes (use defaults) */
    (char *)title,
    NULL,
    NULL};
#endif
static struct NewWindow param_dialog = {
    WINX-200-1, 30,
    200, 50,
    0, 1,
    IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY,
    WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_ACTIVATE,
    NULL, NULL,
    (char *)"Depth:",
    NULL, NULL,
    0, 0,
    WINX, WINY,
    CUSTOMSCREEN};

static struct NewWindow menu_window = {
    WINX-200-1, 10,
    200, 150,
    0, 1,
    IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY,
    WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_ACTIVATE | WFLG_CLOSEGADGET,
    NULL, NULL,
    (char *)"Menu:",
    NULL, NULL,
    0, 0,
    WINX, WINY,
    CUSTOMSCREEN};

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
    MoveSprite(NULL, &sprite_ul, -1, 20 / SCMOUSE);

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
    MoveSprite(NULL, &sprite_lr, WINX / SCMOUSE - 16, WINY / SCMOUSE + 4);
}

void *anim_thread(void *arg)
{
    log_msg("%s: starting animation thread...\n", __FUNCTION__);
    Delay(20);
    while(animation)
    {
        pthread_mutex_lock(&anim_ctrl);
        run_stepAnimation();
        pthread_mutex_unlock(&anim_ctrl);
        Delay(2);
    }
    pthread_mutex_unlock(&anim_ctrl);
    log_msg("%s: terminating animation thread.\n", __FUNCTION__);
    return NULL;
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
        WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE,
        NULL, NULL,
        (char *)title,
        myScreen, NULL,
        0, 0,
        WINX, WINY,
        CUSTOMSCREEN};
    myWindow = OpenWindow(&winlayout);
    rp = myWindow->RPort;
    param_dialog.Screen = myScreen;
    menu_window.Screen = myScreen;

    if (0 == OpenDevice("console.device",-1,(struct IORequest *)&ioreq,0)) {
        ConsoleDevice = (struct Library *)ioreq.io_Device;  
    }
    run_setupDisplay(myWindow, 0, NULL);
    run_setupAnimation(myWindow);
#ifdef PTHREADS    
    pthread_t ath;
    static pthread_attr_t pattr;
    static char *anim_stack[4096];
    pthread_attr_init(&pattr);
    pthread_attr_setstack(&pattr, anim_stack, 4096);
    if (pthread_create(&ath, &pattr, anim_thread, NULL) != 0)
        log_msg("%s: couldn't start animation thread\n", __FUNCTION__);
    pthread_detach(ath);
#endif    
}

/* Convert RAWKEYs into VANILLAKEYs, also shows special keys like HELP, Cursor Keys,
** FKeys, etc.  It returns:
**   -2 if not a RAWKEY event.
**   -1 if not enough room in the buffer, try again with a bigger buffer.
**   otherwise, returns the number of characters placed in the buffer.
*/
LONG deadKeyConvert(struct IntuiMessage *msg, UBYTE *kbuffer,
                    LONG kbsize, struct KeyMap *kmap, struct InputEvent *ievent)
{
    if (msg->Class != IDCMP_RAWKEY)
        return (-2);
    ievent->ie_Class = IECLASS_RAWKEY;
    ievent->ie_Code = msg->Code;
    ievent->ie_Qualifier = msg->Qualifier;
    ievent->ie_position.ie_addr = *((APTR *)msg->IAddress);

    return (RawKeyConvert(ievent, (STRPTR)kbuffer, kbsize, kmap));
}

char fetch_key(struct IntuiMessage *pIMsg, struct Window *win)
{
    struct InputEvent *ievent;
    struct IntuiMessage *m;
    UBYTE buffer[8];
    char ret = '\0';

    ievent = (struct InputEvent *)AllocMem(sizeof(struct InputEvent), MEMF_CLEAR);
    if (ievent)
    {
        if (!(pIMsg->Code & 0x80))
        {
            deadKeyConvert(pIMsg, buffer, 7, NULL, ievent);
            ret = buffer[0];
            while (1) // wait for up-key event
            {
                WaitPort(win->UserPort);
                if ((m = (struct IntuiMessage *)GetMsg(win->UserPort)) != NULL)
                {
                    ReplyMsg((struct Message *) m);
                    if ((m->Class == IDCMP_RAWKEY) &&
                        (m->Code & 0x80))
                    break;
                }
            }
        }
        FreeMem(ievent, sizeof(struct InputEvent));
    }
    return ret;
}

int amiga_setpixel(void *not_used, int x, int y, int col)
{
    //    log_msg("%s: %dx%d->%d\n", __FUNCTION__, x, y, col);
    pthread_mutex_lock(&logmutex);
    SetAPen(rp, col);
    WritePixel(rp, x, y + 10);
    //    Move(rp, x, y+10);
    //    Draw(rp, WINX, y + 10);
    pthread_mutex_unlock(&logmutex);
    struct Message *pMsg;
    int ret = 0;

    if ((pMsg = GetMsg(myWindow->UserPort)) != NULL)
    {
        struct IntuiMessage *pIMsg = (struct IntuiMessage *)pMsg;
        ReplyMsg(pMsg);
        switch (pIMsg->Class)
        {
        case IDCMP_MOUSEBUTTONS:
        case IDCMP_RAWKEY:
            ret = 1;
            while (1) // loop until button up is seen
            {
                pMsg = GetMsg(myWindow->UserPort);
                if (pMsg)
                {
                    ReplyMsg(pMsg);

                    pIMsg = (struct IntuiMessage *)pMsg;
                    if (pIMsg->Code == SELECTUP)
                        goto out;
                    if (pIMsg->Code & 0x80)
                        goto out;
                }
            }
            break;
        case IDCMP_CLOSEWINDOW:
            ret = 1;
            break;
        default:
            //log_msg("%s: class = %ld\n", __FUNCTION__, pIMsg->Class);
            break;
        }
    }
out:
    return ret;
}

int fetch_param(void)
{
    struct Window *paramd = OpenWindow(&param_dialog);
    bool closewin = FALSE;
    long data;
    int new_iter = -1;
    char buf[32];
    memset(buf, 0, 32);
    struct IntuiText reqtext = { 1, 0, JAM1, 0, 0, NULL, (STRPTR) "Depth: ", NULL};
    struct Requester req;
    struct StringInfo reqstringinfo = {
        &buf[0], NULL,
        0, 6,
        0, 
        0, 0, 0, 0, 0,
        NULL,
        iter,
        NULL};
    struct Gadget gad = {
        NULL,
        0, 0,
        180, 35,
        0,
        GACT_ENDGADGET | GACT_STRINGLEFT | GACT_LONGINT,
        GTYP_REQGADGET | GTYP_STRGADGET,
        NULL, NULL,
        NULL, /* text */
        0,
        &reqstringinfo,
        0,
        &data};

    sprintf(buf, "%d", MAX_ITER);
    InitRequester(&req);
    req.LeftEdge = 10;
    req.TopEdge = 30;
    req.Width = 200;
    req.Height = 40;
    req.ReqText = &reqtext;
    req.ReqGadget = &gad;
    up:     
    if (!Request(&req, paramd))
        log_msg("req failed.\n");
   
    while (closewin == FALSE) {
        WaitPort(paramd->UserPort);
        struct Message *pMsg;
        while ((pMsg = GetMsg(paramd->UserPort)) != NULL)
        {
            struct IntuiMessage *pIMsg = (struct IntuiMessage *)pMsg;
            //log_msg("%s: class = %ld\n", __FUNCTION__, pIMsg->Class);
            switch (pIMsg->Class)
            {
            case IDCMP_CLOSEWINDOW:
                closewin = TRUE;
                break;
            case IDCMP_RAWKEY:
                //log_msg("%s: RAWKEY event2: buf = %s\n", __FUNCTION__, buf);
                if (pIMsg->Code & 0x80) // only when key-up is seen.
                {
                    new_iter = strtol(buf, NULL, 10);
                    if ((errno != 0) ||
                        (new_iter < 16) || (new_iter > 1024))
                    {
                        DisplayBeep(myScreen);
                        log_msg("%s: iter out of range: %d\n", __FUNCTION__, new_iter);
                        goto up;
                    }
                    log_msg("%s: iter set to %d\n", __FUNCTION__, new_iter);
                    iter = new_iter;
                    closewin = TRUE;
                }
                break;
            default:
                break;
            }
            ReplyMsg(pMsg);
        }
    }
    EndRequest(&req, paramd);
    CloseWindow(paramd);
    return 0;
}


int show_menu(void)
{
    struct Window *menu = OpenWindow(&menu_window);
    bool closewin = FALSE;
    int ret = 1;
    struct IntuiText t = {
        1, 0, JAM1, 5, 5, NULL, 
        NULL, NULL};
    std::vector<STRPTR> menu_str = {(STRPTR)"i...Iteration", (STRPTR)"r...Recalc", (STRPTR)"s...Restart", (STRPTR)"q...Quit"};
    int j = 0;
    for (auto i : menu_str) {
        t.IText = i;
        PrintIText(menu->RPort, &t, 1, 15 + j);
        j += 12;
    }

    while (closewin == FALSE)
    {
        WaitPort(menu->UserPort);
        struct Message *pMsg;
        while ((pMsg = GetMsg(menu->UserPort)) != NULL)
        {
            struct IntuiMessage *pIMsg = (struct IntuiMessage *)pMsg;
            // log_msg("%s: class = %ld\n", __FUNCTION__, pIMsg->Class);
            ReplyMsg(pMsg);
            switch (pIMsg->Class)
            {
            case IDCMP_CLOSEWINDOW:
                closewin = TRUE;
                break;
            case IDCMP_RAWKEY:
                if (!(pIMsg->Code & 0x80))
                {
                    switch (fetch_key(pIMsg, menu))
                    {
                    case 'q':
                        ret = 0;
                        closewin = TRUE;
                        break;
                    case 'i':
                        fetch_param();
                        closewin = FALSE; // keep menu to decide
                        break;
                    case 'r':
                        ret = 2;
                        closewin = TRUE;
                        break;
                    case 's':
                        ret = 3;
                    default:
                        closewin = TRUE;
                        break;
                    }
                }
            default:
                break;
            }
        }
    }
    CloseWindow(menu);
    return ret;
}

void amiga_zoom(mandel<MTYPE> *m)
{
    uint16_t stx = 0, sty = 10;
    bool closewin = FALSE;
    while (closewin == FALSE)
    {
#ifndef PTHREADS
        run_stepAnimation();
#else
        WaitPort(myWindow->UserPort);
#endif
        struct Message *pMsg;
        while ((pMsg = GetMsg(myWindow->UserPort)) != NULL)
        {
            struct IntuiMessage *pIMsg = (struct IntuiMessage *)pMsg;
            ReplyMsg(pMsg);
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
                MoveSprite(NULL, &sprite_lr, pIMsg->MouseX / SCMOUSE - 16, pIMsg->MouseY / SCMOUSE - 4 * SCMOUSE);  // weird
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
                MoveSprite(NULL, &sprite_ul, (stx - 1) / SCMOUSE, (sty + 10) / SCMOUSE);
                break;
            case IDCMP_MOUSEBUTTONS:
                if (pIMsg->Code == SELECTDOWN)
                {
                    stx = pIMsg->MouseX;
                    sty = pIMsg->MouseY;
                    // log_msg("mouse select start: (%d,%d)\n", stx, sty);
                    ReportMouse(TRUE, myWindow);
                    MoveSprite(NULL, &sprite_ul, (pIMsg->MouseX - 1) / SCMOUSE, (pIMsg->MouseY + 10) / SCMOUSE);
                }
                if (pIMsg->Code == SELECTUP)
                {
                    ReportMouse(FALSE, myWindow);
                    MoveSprite(NULL, &sprite_ul, (-1 / SCMOUSE), 20 / SCMOUSE);
                    MoveSprite(NULL, &sprite_lr, (WINX - 16) / SCMOUSE, (WINY + 4) / SCMOUSE);
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
                if (!(pIMsg->Code & 0x80))
                {
                    switch (show_menu())
                    {
                    case 0:
                        closewin = TRUE;
                        break;
                    case 3:
                        m->mandel_presetup(-1.5, -1.0, 0.5, 1.0);
                    case 2:
                    {
                        point_t lu{0, 0}, rd{WINX, WINY};
                        m->select_start(lu);
                        m->select_end(rd);
                        DisplayBeep(myScreen);
                        ReportMouse(FALSE, myWindow);
                    }
                    break;
                    }
                }
                break;
            }
        }
    }

    pthread_mutex_lock(&anim_ctrl);
    animation = false;
    Delay(5);
    pthread_mutex_lock(&anim_ctrl);
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
    pthread_mutex_init(&anim_ctrl, NULL);
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
