/* Graphics Example 1 */
#include <proto/exec.h>
#include <proto/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
//#include <proto/mathieeedoubbas.h>
//#include <proto/mathieeesingbas.h>
//#include <libraries/mathieeesp.h>

#include <proto/graphics.h>
#ifndef KICK1
#include <proto/gadtools.h>
#include <proto/layers.h>
#endif
#include <graphics/sprite.h>

#include <proto/dos.h>
#include <stdio.h>
#define WINX (320 / 2)
#define WINY (256 / 2)

#include "mandellib.h"
struct SimpleSprite sprite_ul = {0};
struct SimpleSprite sprite_lr = {0};

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

/* real boring sprite data */
UWORD __chip sprite_data_ul[ ] = {
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

UWORD __chip sprite_data_lr[ ] = {
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

void mandel_draw(struct RastPort *rp)
{
    int x, y;
    for (x = 0; x < WINX; x++) {
	for (y = 0; y < WINY; y++) {
	    SetAPen(rp, mandel(x, y));
	    Move(rp, x, y+10);
	    Draw(rp, WINX, y + 10);
	}
    }
}

void sprite_setup(struct Screen *myScreen)
{
    WORD sprnum;
    SHORT color_reg;
    struct ViewPort        *viewport;
    viewport = &myScreen->ViewPort;

    sprnum = GetSprite(&sprite_ul, 2);
    printf("sprite ul num = %d\n", sprnum);
    /* Calculate the correct base color register number, */
    /* set up the color registers.                       */
    color_reg = 16 + ((sprnum & 0x06) << 1);
    printf("color_reg=%d\n", color_reg);
    SetRGB4(viewport, color_reg + 1, 0xf,  0x0,  0x0);

    sprite_ul.x = 0;       /* initialize position and size info    */
    sprite_ul.y = 0;       /* to match that shown in sprite_data   */
    sprite_ul.height = 16;  /* so system knows layout of data later */

    /* install sprite data and move sprite to start position. */
    ChangeSprite(NULL, &sprite_ul, (APTR)sprite_data_ul);
    MoveSprite(NULL, &sprite_ul, -1, 20);

    sprnum = GetSprite(&sprite_lr, 3);
    printf("sprite lr num = %d\n", sprnum);
    /* Calculate the correct base color register number, */
    /* set up the color registers.                       */
    color_reg = 16 + ((sprnum & 0x06) << 1);
    printf("color_reg=%d\n", color_reg);
    SetRGB4(viewport, color_reg + 1, 0xf,  0x0,  0x0);

    sprite_lr.x = 0;       /* initialize position and size info    */
    sprite_lr.y = 0;       /* to match that shown in sprite_data   */
    sprite_lr.height = 16;  /* so system knows layout of data later */

    /* install sprite data and move sprite to start position. */
    ChangeSprite(NULL, &sprite_lr, (APTR)sprite_data_lr);
    MoveSprite(NULL, &sprite_lr, WINX - 16, WINY + 4);
}   

int main(void) {
    struct Window *myWindow;
    struct RastPort *rp;
    int closewin = FALSE;

    printf("hello mandelbrot world.\n");

    IntuitionBase = (struct IntuitionBase *)OpenLibrary((unsigned char *)"intuition.library", 0);
    GfxBase = (struct GfxBase *)OpenLibrary((unsigned char *)"graphics.library", 0);
    printf("openlib returned: %p, %p\n", IntuitionBase, GfxBase);
    
    struct NewScreen Screen1 = {
	0, 0, 320, 320, 6,             /* Screen of 640 x 480 of depth 8 (2^8 = 256 colours)    */
	DETAILPEN, BLOCKPEN,
	EXTRA_HALFBRITE,                     /* see graphics/view.h for view modes */
	CUSTOMSCREEN,              /* Screen types */
	NULL,                      /* Text attributes (use defaults) */
	(unsigned char *)"pottendo's screen", 
	NULL,
	NULL
    };
    struct Screen *myScreen;
    myScreen = OpenScreen(&Screen1);
    ScreenToFront(myScreen);
    ShowTitle(myScreen, TRUE);
    MakeScreen(myScreen);

    sprite_setup(myScreen);
    
#if 1
    char *kick;
    kick = "Mandelbrot";
    struct NewWindow winlayout = {
	0*WINX/4, 0*WINY/4 + 10,
	WINX, WINY + 10,
	0,1, 
	IDCMP_CLOSEWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE,
	WFLG_SIZEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE,
	NULL, NULL,
	(unsigned char *) kick,
	myScreen, NULL,
	0,0,
	WINX,WINY,
	CUSTOMSCREEN
    };
    myWindow = OpenWindow(&winlayout);
#else
    char  Title[] = "Kickstart > 1";
    ULONG WinTags[] = {
	WA_Width,    	WINX,
	WA_Height,   	WINY,
	WA_Title,    	(ULONG)&Title,
	WA_Activate, 	1,
	WA_IDCMP,	IDCMP_CLOSEWINDOW,
	WA_CloseGadget,	1,
	TAG_END
    };
    myWindow = OpenWindowTagList( NULL, (struct TagItem *)&WinTags );
#endif
    
    /* Get Window's Rastport */
    rp = myWindow->RPort;

#if 0
    int ix = WINX / 64;
    int iy = WINY / 64;
    c = 0;
    for (y = 0; y < WINY; y+=iy) {
	for (x = 0; x < WINX; x+=ix) {
	    SetAPen(rp, c);
	    RectFill(rp, x, y, x+ix, y+iy);
	    c++;
	}
    }
    Delay(500);
#endif
    mandel_init(WINX, WINY);
    mandel_draw(rp);
    DisplayBeep(myScreen);
		    
#if 1
    int stx = 0, sty = 0;
    while (closewin == FALSE) {
	WaitPort( myWindow->UserPort );
	
	struct Message * pMsg;
	while ( (pMsg = GetMsg( myWindow->UserPort )) != NULL)	
	{
	    struct IntuiMessage * pIMsg = (struct IntuiMessage *)pMsg;
	    
	    switch ( pIMsg->Class )
	    {
	    case IDCMP_CLOSEWINDOW :
		closewin = TRUE;
		break;
	    case IDCMP_MOUSEMOVE:
		if (pIMsg->MouseX >= WINX) break;
		if (pIMsg->MouseY >= WINY + 10) break;
		if (pIMsg->MouseY < 10) break;
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
		if (stx >= pIMsg->MouseX) stx = pIMsg->MouseX;
		if (sty >= pIMsg->MouseY) sty = pIMsg->MouseY;
		MoveSprite(NULL, &sprite_ul, stx - 1, sty + 10); 
		break;
	    case IDCMP_MOUSEBUTTONS:
		if (pIMsg->Code == SELECTDOWN) {
		    stx = pIMsg->MouseX;
		    sty = pIMsg->MouseY;
		    printf("mouse select start: (%d,%d)\n", stx, sty);
		    ReportMouse(TRUE, myWindow);
		    MoveSprite(NULL, &sprite_ul, pIMsg->MouseX -1 , pIMsg->MouseY + 10);
		}
		if (pIMsg->Code == SELECTUP) {
		    ReportMouse(FALSE, myWindow);
		    MoveSprite(NULL, &sprite_ul, - 1 , 20);
		    MoveSprite(NULL, &sprite_lr, WINX - 16 , WINY + 4);
		    if ((stx == pIMsg->MouseX) ||
			(sty == pIMsg->MouseY) ||
			(sty < 10) ||
			(pIMsg->MouseX > WINX) ||
			(pIMsg->MouseY > WINY))
		    {
			DisplayBeep(myScreen);
			break;
		    }
		    mandel_update(stx, sty - 10, pIMsg->MouseX, pIMsg->MouseY - 10);
		    mandel_init(WINX, WINY);
		    mandel_draw(rp);
		    DisplayBeep(myScreen);
		}
		break;
	    }
	    ReplyMsg( pMsg );
	}
    }
#else
    struct IntuiMessage *msg;
    ULONG msgClass;
    while (closewin == FALSE) {
	Wait(1L << myWindow->UserPort->mp_SigBit);
	msg = GT_GetIMsg(myWindow->UserPort);
	msgClass = msg->Class;
	GT_ReplyIMsg(msg);
	if (msgClass == IDCMP_CLOSEWINDOW) {
	    closewin = TRUE;
	}
	if (msgClass == IDCMP_REFRESHWINDOW)
	    RefreshWindowFrame(myWindow);
    }
#endif
    CloseWindow(myWindow);
    if (myScreen) CloseScreen(myScreen); /* Close screen using myScreen pointer */
    return(0);
}
 
