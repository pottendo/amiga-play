#include <proto/intuition.h>
#include <proto/graphics.h>
#include <intuition/intuition.h>
#include <graphics/gels.h>

#define GEL_SIZE 4 /* number of lines in the bob */

/* Bob data - two sets that are alternated between. Note that this */
/* data is at the resolution of the screen. */
/* data is 2 planes by 2 words by GEL_SIZE lines */
__chip WORD bob_data1[2 * 2 * GEL_SIZE] =
{
      /* plane 1 */
      0xffff, 0x0003, 0xfff0, 0x0003, 0xfff0, 0x0003, 0xffff, 0x0003,
      /* plane 2 */
      0x3fff, 0xfffc, 0x3ff0, 0x0ffc, 0x3ff0, 0x0ffc, 0x3fff, 0xfffc
};

/* data is 2 planes by 2 words by GEL_SIZE lines */
__chip WORD bob_data2[2 * 2 * GEL_SIZE] =
{
      /* plane 1 */
      0xc000, 0xffff, 0xc000, 0x0fff, 0xc000, 0x0fff, 0xc000, 0xffff,
      /* plane 2 */
      0x3fff, 0xfffc, 0x3ff0, 0x0ffc, 0x3ff0, 0x0ffc, 0x3fff, 0xfffc
};

NEWBOB myNewBob = /* Data for the new bob structure defined in animtools.h    */
{ /* Initial image, WORD width, line height */
      bob_data2, 2, GEL_SIZE,             /* Image depth, plane pick, plane    on off, VSprite flags */
      2, 3, 0, SAVEBACK | OVERLAY,        /* dbuf (0=false), raster depth,    x,y position, hit mask, */
      0, 2, 160, 100, 0,0,                /* me mask */
};
 

struct NewWindow myNewWindow =
   { /* information for the new window */
   80, 20, 400, 150, -1, -1, CLOSEWINDOW | INTUITICKS,
   ACTIVATE | WINDOWCLOSE | WINDOWDEPTH | RMBTRAP,
   NULL, NULL, "Bob", NULL, NULL, 0, 0, 0, 0, WBENCHSCREEN
   };
 
/* Draw the Bobs into the RastPort. */
   VOID bobDrawGList(struct RastPort *rport, struct ViewPort *vport)
   {
       /* This function sorts the gel list by its x,y co-ords, required before DrawGList */ 
       SortGList(rport);
       /* This function draws all the gels including bobs and vsprites */ 
       DrawGList(rport, vport); 
       /* If the GelsList includes true VSprites, MrgCop() and LoadView() here */
       WaitTOF() ; /* Wait for next top of frame refresh interval before continuing */ 
   }

VOID do_Bob(struct Window *win)
   {
      struct Bob *myBob;
      struct GelsInfo *my_ginfo;

   if (NULL == (my_ginfo = setupGelSys(win->RPort, 0x03)))
         return_code = RETURN_WARN;
      else
     {
      /* Create the Bob using makeBob() from Animtools.c to allocate    memory and fill in the required information for Bob structure
           to create the Bob object */
       if (NULL == (myBob = makeBob(&myNewBob)))
             return_code = RETURN_WARN;
       else
       {
            /* Add the bob to the window's rastport ready for display */
            AddBob(myBob, win->RPort);
            /* Display the gels (bobs) in the given window and viewport */
            bobDrawGList(win->RPort, ViewPortAddress(win));
            /* do animation in this function */
            process_window(win, myBob);
           /* When done, remove bob from system */
           RemBob(myBob);
           /* Alternatively, use RemIBob(myBob, win->RPort, ViewPortAddress(win)); */
           /* Update the display to remove it from the screen */
           bobDrawGList(win->RPort, ViewPortAddress(win));
          /* Free up any memory uses by bob  */ 
           freeBob(myBob, myNewBob.nb_RasDepth);
       }
        cleanupGelSys(my_ginfo,win->RPort);
      }
   }
 
VOID process_window(struct Window *win, struct Bob *myBob)
{
   struct IntuiMessage *msg;

/* FOREVER is equivalent to an empty for() loop that never ends */
   FOREVER {
     /* Wait for a signal bit to be returned from the window */
     Wait(1L << win->UserPort->mp_SigBit);
     /* If a message is received from window then process it */
     while (NULL != (msg = (struct IntuiMessage *)GetMsg(win->UserPort)))
     {
       /* only CLOSEWINDOW and INTUITICKS are active */
       if (msg->Class == CLOSEWINDOW)
      {
          /* If the CloseWindows gadget is pressed, then reply and exit this function */
           ReplyMsg((struct Message *)msg);
           return;
       }
      /* Must be INTUITICKS: change x and y values on the fly. Note:
      ** do not have to add window offset, Bob is relative to the
      ** window (sprite relative to screen).
      */
      myBob->BobVSprite->X = msg->MouseX + 20;
      myBob->BobVSprite->Y = msg->MouseY + 1;
      ReplyMsg((struct Message *)msg);
     }
     /* after getting a message, change image data on the fly */
      myBob->BobVSprite->ImageData = (myBob->BobVSprite->ImageData == bob_data1) ? bob_data2 :    bob_data1;
     InitMasks(myBob->BobVSprite);      /* set up masks for new image */
     bobDrawGList(win->RPort, ViewPortAddress(win));
    }
   }
 