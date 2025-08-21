// Want to be KS1.3 -> V34.xx
// #define INTUI_V36_NAMES_ONLY

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <proto/dos.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parport.h"

#define BUFSIZE 8000
static unsigned char str[BUFSIZE];

static void busywrite(int del)
{
    int j = 0;
    struct IOExtPar *ParallelIO = nullptr;
    for (int i = 0; i < BUFSIZE; i++)
        str[i] = (unsigned char)(i & 0xff);
    ParallelIO = open_parport();
    if (ParallelIO)
    {
        status_parport(ParallelIO, __FUNCTION__);
        while (1)
        {
            memset(str, (unsigned char) (j++) &0xff, BUFSIZE);
            switch(write_parport(ParallelIO, str, BUFSIZE))
            {
            case -EINTR:
                printf("%s: EINTR received\n", __FUNCTION__);
                goto done;
                break;
            case -EBUSY:
                printf("%s: EBUSY received\n", __FUNCTION__);
                Delay(13);
                break;
            default:
                Delay(del);
            }
        }
    done:
        close_parport(ParallelIO);
    }
    printf("%s done\n", __FUNCTION__);
}

static void busyread(int len)
{
    struct IOExtPar *ParallelIO = nullptr;
    int res, i = 0;
    ParallelIO = open_parport();
    if (ParallelIO)
    {
        printf("%s: reading parport...\n", __FUNCTION__);
        while (1)
        {
            // status_parport(ParallelIO, __FUNCTION__);
            if ((res = read_parport(ParallelIO, str, len)) < 0)
            {
                if (res == -EINTR)
                    printf("%s: EINTR received\n", __FUNCTION__);
                else
                    printf("%s: read failed, received: %d\n", __FUNCTION__, errno);
                break;
            }
            if ((i++ % 8) == 0)
            {
                char t[16];
                char outstr[BUFSIZE * 4];
                outstr[0] = '\0';
                printf("%s: read %d bytes\n", __FUNCTION__, res * 8);
                for (int i = 0; i < 16; i++)
                {
                    snprintf(t, 16, "%02x ", str[i]);
                    strcat(outstr, t);
                }
                printf("%s\n", outstr);
                outstr[0] = '\0';
            }
        }
        close_parport(ParallelIO);
    }
    printf("%s done\n", __FUNCTION__);
}

static void statparport(void)
{
    struct IOExtPar *ParallelIO = nullptr;
    ParallelIO = open_parport();
    if (ParallelIO)
    {
        status_parport(ParallelIO, __FUNCTION__);
        close_parport(ParallelIO);
    }
    printf("%s done\n", __FUNCTION__);
}
// ------------------------------- UI & Main --------------------


/*  These values are based on the ROM font Topaz8. Adjust these  */
/*  values to correctly handle the screen's current font.        */
#define MENWIDTH (56 + 8) /* Longest menu item name * font width */
                          /* + 8 pixels for trim                 */
#define MENHEIGHT (10)    /* Font height + 2 pixels              */

struct Library *GfxBase;
struct Library *IntuitionBase;

/* To keep this example simple, we'll hard-code the font used for menu */
/* items.  Algorithmic layout can be used to handle arbitrary fonts.   */
/* Under Release 2, GadTools provides font-sensitive menu layout.      */
/* Note that we still must handle fonts for the menu headers.          */
struct TextAttr Topaz80 =
    {
        (STRPTR) "topaz.font", 8, 0, 0};

struct IntuiText menuIText[] =
    {
        {0, 1, JAM2, 0, 1, &Topaz80, (STRPTR) "Echo", NULL},
        {0, 1, JAM2, 0, 1, &Topaz80, (STRPTR) "Dump: Amiga -> ESP", NULL},
        {0, 1, JAM2, 0, 1, &Topaz80, (STRPTR) "Dump: ESP -> Amiga", NULL},
        {0, 1, JAM2, 0, 1, &Topaz80, (STRPTR) "Mandelbrot", NULL},
        {0, 1, JAM2, 0, 1, &Topaz80, (STRPTR) "Parport status", NULL},
        {0, 1, JAM2, 0, 1, &Topaz80, (STRPTR) "Quit", NULL}};

#if 0
struct MenuItem submenu1[] =
{
    { /* Draft  */
    &submenu1[1], MENWIDTH-2,  -2 ,            MENWIDTH, MENHEIGHT,
    ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0, (APTR)&menuIText[3], NULL, NULL, NULL, NULL
    },
    { /* NLQ    */
    NULL,         MENWIDTH-2, MENHEIGHT-2, MENWIDTH, MENHEIGHT,
    ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0, (APTR)&menuIText[4], NULL, NULL, NULL, NULL
    }
};
#endif
struct MenuItem menu1[] = {
        {/* Echo */
         &menu1[1], 0, 0, MENWIDTH, MENHEIGHT,
         ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
         0, (APTR)&menuIText[0], NULL, 0, NULL, 0},
        {/* Dump1    */
         &menu1[2], 0, MENHEIGHT, MENWIDTH, MENHEIGHT,
         ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
         0, (APTR)&menuIText[1], NULL, 1, NULL, 0},
        {/* Dump2   */
         &menu1[3], 0, 2 * MENHEIGHT, MENWIDTH, MENHEIGHT,
         ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
         0, (APTR)&menuIText[2], NULL, 2, NULL, 0},
        {/* Mandelbrot   */
         &menu1[4], 0, 3 * MENHEIGHT, MENWIDTH, MENHEIGHT,
         ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
         0, (APTR)&menuIText[3], NULL, 3, NULL, 0},
        {/* Status    */
         &menu1[5], 0, 4 * MENHEIGHT, MENWIDTH, MENHEIGHT,
         ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
         0, (APTR)&menuIText[4], NULL, 4, NULL, 0},
        {/* Quit */
         NULL, 0, 5 * MENHEIGHT, MENWIDTH, MENHEIGHT,
         ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
         0, (APTR)&menuIText[5], NULL, 5, NULL, 0},
};

/* We only use a single menu, but the code is generalizable to */
/* more than one menu.                                         */
#define NUM_MENUS 1

STRPTR menutitle[NUM_MENUS] = {(STRPTR) "Project"};

struct Menu menustrip[NUM_MENUS] = {
        {
            NULL,         /* Next Menu          */
            0, 0,         /* LeftEdge, TopEdge, */
            0, MENHEIGHT, /* Width, Height,     */
            MENUENABLED,  /* Flags              */
            NULL,         /* Title              */
            &menu1[0]     /* First item         */
        }};

struct NewWindow mynewWindow = {
        40, 40, 300, 100, 0, 1, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK,
        WFLG_DRAGBAR | WFLG_ACTIVATE | WFLG_CLOSEGADGET, NULL, NULL,
        (STRPTR) "Menu Test Window", NULL, NULL, 0, 0, 0, 0, WBENCHSCREEN};

/* our function prototypes */
VOID handleWindow(struct Window *win, struct Menu *menuStrip);

/*
**   Wait for the user to select the close gadget.
*/
VOID handleWindow(struct Window *win, struct Menu *menuStrip)
{
    struct IntuiMessage *msg;
    SHORT done;
    ULONG classx;
    UWORD menuNumber;
    UWORD menuNum;
    UWORD itemNum;
    UWORD subNum;
    struct MenuItem *item;

    done = FALSE;
    while (FALSE == done)
    {
        /* we only have one signal bit, so we do not have to check which
        ** bit broke the Wait().
        */
        Wait(1L << win->UserPort->mp_SigBit);

        while ((FALSE == done) &&
               (msg = (struct IntuiMessage *)GetMsg(win->UserPort)))
        {
            classx = msg->Class;
            if (classx == IDCMP_MENUPICK)
                menuNumber = msg->Code;

            switch (classx)
            {
            case IDCMP_CLOSEWINDOW:
                done = TRUE;
                break;
            case IDCMP_MENUPICK:
                while ((menuNumber != MENUNULL) && (!done))
                {
                    item = ItemAddress(menuStrip, menuNumber);

                    /* process this item
                    ** if there were no sub-items attached to that item,
                    ** SubNumber will equal NOSUB.
                    */
                    menuNum = MENUNUM(menuNumber);
                    itemNum = ITEMNUM(menuNumber);
                    subNum = SUBNUM(menuNumber);

                    /* Note that we are printing all values, even things
                    ** like NOMENU, NOITEM and NOSUB.  An application should
                    ** check for these cases.
                    */
                    printf("IDCMP_MENUPICK: menu %d, item %d, sub %d\n",
                           menuNum, itemNum, subNum);
                    if (menuNum == 0)
                    {
                        switch(itemNum) {
                            case 1:
                                busywrite(0);
                                break;
                            case 2:
                                busyread(BUFSIZE);
                                break;
                            case 4:
                                statparport();
                                break;
                            case 0:
                            case 3:
                                printf("not implemented\n");
                            default:
                                break;
                        }
                    }
                    /* This one is the quit menu selection...
                    ** stop if we get it, and don't process any more.
                    */
                    if ((menuNum == 0) && (itemNum == 5))
                        done = TRUE;

                    menuNumber = item->NextSelect;
                }
                break;
            }
            ReplyMsg((struct Message *)msg);
        }
    }
}

int main(int argc, char **argv)
{
    struct Window *win = NULL;
    UWORD left, m;

    /* Open the Graphics Library */
    GfxBase = OpenLibrary("graphics.library", 33);
    if (GfxBase)
    {
        /* Open the Intuition Library */
        IntuitionBase = OpenLibrary("intuition.library", 33);
        if (IntuitionBase)
        {
            if ((win = OpenWindow(&mynewWindow)) != nullptr)
            {
                left = 2;
                for (m = 0; m < NUM_MENUS; m++)
                {
                    menustrip[m].LeftEdge = left;
                    menustrip[m].MenuName = menutitle[m];
                    menustrip[m].Width = TextLength(&win->WScreen->RastPort,
                                                    menutitle[m], strlen(menutitle[m])) +
                                         8;
                    left += menustrip[m].Width;
                }
                if (SetMenuStrip(win, menustrip))
                {
                    printf("successfull MenuStrip\n");
                    handleWindow(win, menustrip);
                    ClearMenuStrip(win);
                }
                CloseWindow(win);
            }
            CloseLibrary(IntuitionBase);
        }
        CloseLibrary(GfxBase);
    }
}
