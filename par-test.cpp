#if 0
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <graphics/gfx.h>
#include <graphics/view.h>
#include <graphics/display.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdio.h>

struct IntuitionBase *IntuitionBase;

struct Window *window;
struct Menu menu;
struct MenuItem item1, item2, item3;
struct IntuiText text1, text2, text3;

// Action functions
void action1() { printf("Menu Item 1 selected: Hello!\n"); }
void action2() { printf("Menu Item 2 selected: Do Something...\n"); }
void action3() { printf("Menu Item 3 selected: Quit.\n"); }
#define IMG_W 640
#define IMG_H 480
#define SCRDEPTH 4
#define SCRMODE (HIRES|LACE)
static char *titlex = "parallel port test";
static struct NewScreen Screen1 = {
    0, 0, IMG_W, IMG_H + 20, SCRDEPTH, /* Screen of 640 x 480 of depth 8 (2^8 = 256 colours)    */
    DETAILPEN, BLOCKPEN,
    SCRMODE, /* see graphics/view.h for view modes */
    WBENCHSCREEN,    /* Screen types */
    NULL,            /* Text attributes (use defaults) */
    (char *)titlex,
    NULL,
    NULL};
/* Settings Item IntuiText */
struct IntuiText SettText[] = {
        {0,1,JAM2,2,         1, NULL, "Sound...",        NULL },
        {0,1,JAM2,CHECKWIDTH,1, NULL, " Auto Save",      NULL },
        {0,1,JAM2,CHECKWIDTH,1, NULL, " Have Your Cake", NULL },
        {0,1,JAM2,CHECKWIDTH,1, NULL, " Eat It Too",     NULL }
    };

struct MenuItem SettItem[] = {
        { /* "Sound..." */
            &SettItem[1], 0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&SettText[0], NULL, NULL, NULL, MENUNULL },
        { /* "Auto Save" (toggle-select, initially selected) */
            &SettItem[2], 0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP|CHECKIT|MENUTOGGLE|CHECKED, 0,
            (APTR)&SettText[1], NULL, NULL, NULL, MENUNULL },
        { /* "Have Your Cake" (initially selected, excludes "Eat It Too") */
            &SettItem[3], 0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP|CHECKIT|CHECKED, 8,
            (APTR)&SettText[2], NULL, NULL, NULL, MENUNULL },
        { /* "Eat It Too" (excludes "Have Your Cake") */
            NULL, 0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP|CHECKIT, 4,
            (APTR)&SettText[3], NULL, NULL, NULL, MENUNULL }
    };
/* Edit Menu Item IntuiText */
struct IntuiText EditText[] = {
        {0,1,JAM2,2,1, NULL, "Cut",       NULL },
        {0,1,JAM2,2,1, NULL, "Copy",      NULL },
        {0,1,JAM2,2,1, NULL, "Paste",     NULL },
        {0,1,JAM2,2,1, NULL, "Erase",     NULL },
        {0,1,JAM2,2,1, NULL, "Undo",      NULL }
    };

/* Edit Menu Items */
struct MenuItem EditItem[] = {
        { /* "Cut" (key-equivalent: 'X') */
            &EditItem[1], 0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&EditText[0], NULL, 'X', NULL, MENUNULL },
        { /* "Copy" (key-equivalent: 'C') */
            &EditItem[2], 0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&EditText[1], NULL, 'C', NULL, MENUNULL },
        { /* "Paste" (key-equivalent: 'V') */
            &EditItem[3], 0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&EditText[2], NULL, 'V', NULL, MENUNULL },
        { /* "Erase" (disabled) */
            &EditItem[4], 0, 0, 0, 0, ITEMTEXT|HIGHCOMP, 0,
            (APTR)&EditText[3], NULL, NULL, NULL, MENUNULL },
        { /* "Undo" MenuItem (key-equivalent: 'Z') */
            NULL, 0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&EditText[4], NULL, 'Z', NULL, MENUNULL }
    };

/* IntuiText for the Print Sub-Items */
struct IntuiText PrtText[] = {
        {0,1, JAM2,2,1, NULL, "NLQ",   NULL },
        {0,1, JAM2,2,1, NULL, "Draft", NULL }
    };

/* Print Sub-Items */
struct MenuItem PrtItem[] = {
        { /* "NLQ" */
            &PrtItem[1], 0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&PrtText[0], NULL, NULL, NULL, MENUNULL },
        { /* "Draft" */
            NULL, 0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&PrtText[1], NULL, NULL, NULL, MENUNULL }
    };
/* Uses the >> character to indicate a sub-menu item.
** This is \273 Octal, 0xBB Hex or Alt-0 from the Keyboard.
**
** NOTE that standard menus place this character at the right margin of the menu box.
** This may be done by using a second IntuiText structure for the single character,
** linking this IntuiText to the first one, and positioning the IntuiText so that the
** character appears at the right margin.  GadTools library will provide the correct behavior.
*/

/* Project Menu Item IntuiText */
struct IntuiText ProjText[] = {
        {0,1, JAM2,2,1, NULL, "New",         NULL },
        {0,1, JAM2,2,1, NULL, "Open...",     NULL },
        {0,1, JAM2,2,1, NULL, "Save",        NULL },
        {0,1, JAM2,2,1, NULL, "Save As...",  NULL },
        {0,1, JAM2,2,1, NULL, "Print     \273", NULL },
        {0,1, JAM2,2,1, NULL, "About...",    NULL },
        {0,1, JAM2,2,1, NULL, "Quit",        NULL }
    };


/* Project Menu Items */
struct MenuItem ProjItem[] = {
        { /* "New" (key-equivalent: 'N' */
            &ProjItem[1],0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&ProjText[0], NULL, 'N', NULL, MENUNULL },
        { /* "Open..." (key-equivalent: 'O') */
            &ProjItem[2],0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&ProjText[1], NULL, 'O', NULL, MENUNULL },
        { /* "Save" (key-equivalent: 'S') */
            &ProjItem[3],0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&ProjText[2], NULL, 'S', NULL, MENUNULL },
        { /* "Save As..." (key-equivalent: 'A') */
            &ProjItem[4],0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&ProjText[3], NULL, 'A', NULL, MENUNULL },
        { /* "Print" (has sub-menu) */
            &ProjItem[5],0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&ProjText[4], NULL, NULL, &PrtItem[0], MENUNULL },
        { /* "About..." */
            &ProjItem[6],0, 0, 0, 0, ITEMTEXT|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&ProjText[5], NULL, NULL, NULL, MENUNULL },
        { /* "Quit" (key-equivalent: 'Q' */
            NULL, 0, 0, 0, 0, ITEMTEXT|COMMSEQ|ITEMENABLED|HIGHCOMP, 0,
            (APTR)&ProjText[6], NULL, 'Q', NULL, MENUNULL }
    };

/* Menu Titles */
struct Menu Menus[] = {
        {&Menus[1],  0, 0, 63, 0, MENUENABLED, "Project",    &ProjItem[0]},
        {&Menus[2], 70, 0, 39, 0, MENUENABLED, "Edit",       &EditItem[0]},
        {NULL,     120, 0, 88, 0, MENUENABLED, "Settings",   &SettItem[0]},
    };

static struct Screen *myScreen;
// Setup menu bar and items
void setup_menu() {
    text1.FrontPen = 1;
    text1.BackPen = 0;
    text1.DrawMode = JAM1;
    text1.LeftEdge = 0;
    text1.TopEdge = 1;
    text1.IText = (STRPTR)"Say Hello";
    text1.NextText = NULL;

    text2 = text1;
    text2.IText = (STRPTR)"Do Action";

    text3 = text1;
    text3.IText = (STRPTR)"Quit";

    item1.NextItem = &item2;
    item1.LeftEdge = 0;
    item1.TopEdge = 0;
    item1.Width = 100;
    item1.Height = 10;
    item1.Flags = ITEMTEXT | ITEMENABLED;
    item1.ItemFill = &text1;
    item1.SelectFill = NULL;
    item1.Command = 0;
    item1.SubItem = NULL;
    item1.NextSelect = MENUNULL;
    //item1.ItemText = &text1;

    item2 = item1;
    item2.ItemFill = &text2;
    item2.Command = 1;
    item2.NextItem = &item3;
    //item2.ItemText = &text2;

    item3 = item1;
    item3.NextItem = NULL;
    item3.Command = 2;
    item3.ItemFill = &text3;
    //item3.ItemText = &text3;

    menu.NextMenu = NULL;
    menu.LeftEdge = 20;
    menu.TopEdge = 1;
    menu.Width = 200;
    menu.Height = 10;
    menu.Flags = MENUENABLED;
    menu.MenuName = (CONST_STRPTR)"File";
    menu.FirstItem = &item1;
}

int main() {
    IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
    if (!IntuitionBase) {
        printf("Cannot open intuition.library!\n");
        return 1;
    }
    myScreen = OpenScreen(&Screen1);
    ScreenToFront(myScreen);
    ShowTitle(myScreen, TRUE);
    setup_menu();
    struct NewWindow nw = {
        0, 0,   // Left, Top
        480, 200, // Width, Height
        0, 1,     // DetailPen, BlockPen
        IDCMP_CLOSEWINDOW | IDCMP_MENUPICK,
        WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET |
            WFLG_ACTIVATE | WFLG_RMBTRAP | WFLG_SMART_REFRESH |
            WFLG_SIMPLE_REFRESH | WFLG_SIZEGADGET,
        NULL,            // Gadget
        NULL,                       // Image
        (STRPTR) "Amiga Menu Demo", // Title
        myScreen,                   // Screen
        NULL,                       // BitMap
        50, 50,                       // MinWidth, MinHeight
        250, 180,                   // MaxWidth, MaxHeight
        WBENCHSCREEN                // Type
    };


    window = OpenWindow(&nw);

    if (!window) {
        printf("Cannot open window!\n");
        CloseLibrary((struct Library *)IntuitionBase);
        return 1;
    }

    SetMenuStrip(window, &menu);
    /* Set up the signals that you want to hear about ... */
    ULONG signalmask = (SIGBREAKF_CTRL_C | 1L << window->UserPort->mp_SigBit);
    ULONG signals;
    /* And wait to hear from your signals */
    while (1)
    {
        signals = Wait(signalmask);
        if (signals & signalmask)
            break;
    };

    BOOL running = TRUE;
    while (running)
    {
        struct IntuiMessage *msg;
        WaitPort(window->UserPort);
        while ((msg = (struct IntuiMessage *)GetMsg(window->UserPort))) {
            if (msg->Class == IDCMP_CLOSEWINDOW) {
                running = FALSE;
            } else if (msg->Class == IDCMP_MENUPICK) {
                UWORD code = msg->Code;
                while (code != MENUNULL) {
                    UWORD item = ITEMNUM(code);
                    switch (item) {
                        case 0: action1(); break;
                        case 1: action2(); break;
                        case 2: action3(); running = FALSE; break;
                    }
                    code = msg->Code >> 16;
                }
            }
            ReplyMsg((struct Message *)msg);
        }
    }

    ClearMenuStrip(window);
    CloseWindow(window);
    CloseScreen(myScreen);
    CloseLibrary((struct Library *)IntuitionBase);

    return 0;
}
#else
/* simplemenu.c - Execute me to compile me with SAS C 5.10
LC -b1 -cfistq -v -y -j73 simplemenu.c
Blink FROM LIB:c.o,simplemenu.o TO simplemenu LIBRARY LIB:LC.lib,LIB:Amiga.lib
quit

** simplemenu.c: how to use the menu system with a window under all OS versions.
*/
#define INTUI_V36_NAMES_ONLY

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include <stdio.h>
#include <string.h>

#ifdef LATTICE
int CXBRK(void)    { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

/*  These values are based on the ROM font Topaz8. Adjust these  */
/*  values to correctly handle the screen's current font.        */
#define MENWIDTH  (56+8)  /* Longest menu item name * font width */
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
    "topaz.font", 8, 0, 0
};

struct IntuiText menuIText[] =
{
    { 0, 1, JAM2, 0, 1, &Topaz80, "Open...",  NULL },
    { 0, 1, JAM2, 0, 1, &Topaz80, "Save",     NULL },
    { 0, 1, JAM2, 0, 1, &Topaz80, "Print \273",  NULL },
    { 0, 1, JAM2, 0, 1, &Topaz80, "Draft",    NULL },
    { 0, 1, JAM2, 0, 1, &Topaz80, "NLQ",      NULL },
    { 0, 1, JAM2, 0, 1, &Topaz80, "Quit",     NULL }
};

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

struct MenuItem menu1[] =
{
    { /* Open... */
    &menu1[1], 0, 0,            MENWIDTH, MENHEIGHT,
    ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0, (APTR)&menuIText[0], NULL, NULL, NULL, NULL
    },
    { /* Save    */
    &menu1[2], 0,  MENHEIGHT ,  MENWIDTH, MENHEIGHT,
    ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0, (APTR)&menuIText[1], NULL, NULL, NULL, NULL
    },
    { /* Print   */
    &menu1[3], 0, 2*MENHEIGHT , MENWIDTH, MENHEIGHT,
    ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0, (APTR)&menuIText[2], NULL, NULL, &submenu1[0] , NULL
    },
    { /* Quit    */
    NULL, 0, 3*MENHEIGHT , MENWIDTH, MENHEIGHT,
    ITEMTEXT | MENUTOGGLE | ITEMENABLED | HIGHCOMP,
    0, (APTR)&menuIText[5], NULL, NULL, NULL, NULL
    },
};

/* We only use a single menu, but the code is generalizable to */
/* more than one menu.                                         */
#define NUM_MENUS 1

STRPTR menutitle[NUM_MENUS] =  {   "Project"   };

struct Menu menustrip[NUM_MENUS] =
{
    {
    NULL,                    /* Next Menu          */
    0, 0,                    /* LeftEdge, TopEdge, */
    0, MENHEIGHT,            /* Width, Height,     */
    MENUENABLED,             /* Flags              */
    NULL,                    /* Title              */
    &menu1[0]                /* First item         */
    }
};

struct NewWindow mynewWindow =
{
40,40, 300,100, 0,1, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK,
WFLG_DRAGBAR | WFLG_ACTIVATE | WFLG_CLOSEGADGET, NULL,NULL,
"Menu Test Window", NULL,NULL,0,0,0,0,WBENCHSCREEN
};

/* our function prototypes */
VOID handleWindow(struct Window *win, struct Menu *menuStrip);

/*      Main routine.         */
/*                            */
int main(int argc, char **argv)
{
struct Window *win=NULL;
UWORD left, m;

/* Open the Graphics Library */
GfxBase = OpenLibrary("graphics.library",33);
if (GfxBase)
    {
    /* Open the Intuition Library */
    IntuitionBase = OpenLibrary("intuition.library", 33);
    if (IntuitionBase)
        {
        if ( win = OpenWindow(&mynewWindow) )
            {
            left = 2;
            for (m = 0; m < NUM_MENUS; m++)
                {
                menustrip[m].LeftEdge = left;
                menustrip[m].MenuName = menutitle[m];
                menustrip[m].Width = TextLength(&win->WScreen->RastPort,
                    menutitle[m], strlen(menutitle[m])) + 8;
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

    while ( (FALSE == done) &&
            (msg = (struct IntuiMessage *)GetMsg(win->UserPort)))
        {
        classx = msg->Class;
        if(classx == IDCMP_MENUPICK)   menuNumber = msg->Code;

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
                    subNum  = SUBNUM(menuNumber);

                    /* Note that we are printing all values, even things
                    ** like NOMENU, NOITEM and NOSUB.  An application should
                    ** check for these cases.
                    */
                    printf("IDCMP_MENUPICK: menu %d, item %d, sub %d\n",
                        menuNum, itemNum, subNum);

                    /* This one is the quit menu selection...
                    ** stop if we get it, and don't process any more.
                    */
                    if ((menuNum == 0) && (itemNum == 4))
                        done = TRUE;

                    menuNumber = item->NextSelect;
                    }
                break;
            }
        ReplyMsg((struct Message *)msg);
        }
    }
}



#endif