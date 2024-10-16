/* Copyright (c) 1990 Commodore-Amiga, Inc.
 *
 * This example is provided in electronic form by Commodore-Amiga, Inc. for
 * use with the 1.3 revisions of the Addison-Wesley Amiga reference manuals. 
 * The 1.3 Addison-Wesley Amiga Reference Manual series contains additional
 * information on the correct usage of the techniques and operating system
 * functions presented in this example.  The source and executable code of
 * this example may only be distributed in free electronic form, via bulletin
 * board or as part of a fully non-commercial and freely redistributable
 * diskette.  Both the source and executable code (including comments) must
 * be included, without modification, in any copy.  This example may not be
 * published in printed form or distributed with any commercial product.
 * However, the programming techniques and support routines set forth in
 * this example may be used in the development of original executable
 * software products for Commodore Amiga computers.
 * All other rights reserved.
 * This example is provided "as-is" and is subject to change; no warranties
 * are made.  All use is at your own risk.  No liability or responsibility
 * is assumed.
 */

/* animtools.c 19oct89 original code by Dave Lucas.
** rework by (CATS)
**
** This file is a collection of tools which are used with the VSprite, Bob
** and Animation system software. It is intended as a useful EXAMPLE, and
** while it shows what must be done, it is not the only way to do it.
** If Not Enough Memory, or error return, each cleans up after itself
** before returning.
**
** NOTE:  these routines assume a very specific structure to the
** gel lists.  make sure that you use the correct pairs together
** (i.e. makeOb()/freeOb(), etc.)
**
** lattice c 5.04
** lc -b1 -cfist -v -y animtools.c
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gels.h>
#include <graphics/clip.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/gfxbase.h>

#include "animtools.h"
#include "animtools_proto.h"

//#include <proto/all.h>

/*-------------------------------------------------------------
** setup the gels system.  After this call is made you can use
** vsprites, bobs, anim comps, and anim obs.
**
** note that this links the GelsInfo structure into the rast port,
** and calls InitGels().
**
** all resources are properly freed on failure.
**
** It uses information in your RastPort structure to establish
** boundary collision defaults at the outer edges of the raster.
**
** This routine sets up for everything - collision detection and all.
**
** You must already have run LoadView before ReadyGelSys is called.
*/
struct GelsInfo *setupGelSys(struct RastPort *rPort, BYTE reserved)
{
struct GelsInfo *gInfo;
struct VSprite  *vsHead;
struct VSprite  *vsTail;

if (NULL != (gInfo =
	(struct GelsInfo *)AllocMem((LONG)sizeof(struct GelsInfo), MEMF_CLEAR)))
	{
	if (NULL != (gInfo->nextLine =
		(WORD *)AllocMem((LONG)sizeof(WORD) * 8, MEMF_CLEAR)))
		{
		if (NULL != (gInfo->lastColor =
			(WORD **)AllocMem((LONG)sizeof(LONG) * 8, MEMF_CLEAR)))
			{
			if (NULL != (gInfo->collHandler =
				(struct collTable *)AllocMem((LONG)sizeof(struct collTable),
					MEMF_CLEAR)))
				{
				if (NULL != (vsHead = (struct VSprite *)AllocMem(
					(LONG)sizeof(struct VSprite), MEMF_CLEAR)))
					{
					if (NULL != (vsTail = (struct VSprite *)AllocMem(
						(LONG)sizeof(struct VSprite), MEMF_CLEAR)))
						{
						gInfo->sprRsrvd	  = reserved;
						gInfo->leftmost	  = 1;
						gInfo->rightmost  =
							(rPort->BitMap->BytesPerRow << 3) - 1;
						gInfo->topmost	  = 1;
						gInfo->bottommost = rPort->BitMap->Rows - 1;

						rPort->GelsInfo = gInfo;

						InitGels(vsHead, vsTail, gInfo);

						return(gInfo);
						}
					FreeMem(vsHead, (LONG)sizeof(*vsHead));
					}
				FreeMem(gInfo->collHandler, (LONG)sizeof(struct collTable));
				}
			FreeMem(gInfo->lastColor, (LONG)sizeof(LONG) * 8);
			}
		FreeMem(gInfo->nextLine, (LONG)sizeof(WORD) * 8);
		}
	FreeMem(gInfo, (LONG)sizeof(*gInfo));
	}
return(NULL);
}

/*-------------------------------------------------------------
** free all of the stuff allocated by setupGelSys().
** only call this routine if setupGelSys() returned successfully.
** the GelsInfo structure is the one returned by setupGelSys().
**
** It also unlinks the GelsInfo from the RastPort.
*/
VOID cleanupGelSys(struct GelsInfo *gInfo, struct RastPort *rPort)
{
rPort->GelsInfo = NULL;

FreeMem(gInfo->collHandler, (LONG)sizeof(struct collTable));
FreeMem(gInfo->lastColor, (LONG)sizeof(LONG) * 8);
FreeMem(gInfo->nextLine, (LONG)sizeof(WORD) * 8);
FreeMem(gInfo->gelHead, (LONG)sizeof(struct VSprite));
FreeMem(gInfo->gelTail, (LONG)sizeof(struct VSprite));
FreeMem(gInfo, (LONG)sizeof(*gInfo));
}

/*-------------------------------------------------------------
** create a VSprite from the information given in nVSprite.
** use freeVSprite() to free this gel.
*/
struct VSprite *makeVSprite(NEWVSPRITE *nVSprite)
{
struct VSprite *vsprite;
LONG			line_size;
LONG			plane_size;

line_size = (LONG)sizeof(WORD) * nVSprite->nvs_WordWidth;
plane_size = line_size * nVSprite->nvs_LineHeight;

if (NULL != (vsprite =
	(struct VSprite *)AllocMem((LONG)sizeof(struct VSprite), MEMF_CLEAR)))
	{
	if (NULL != (vsprite->BorderLine =
		(WORD *)AllocMem(line_size, MEMF_CHIP)))
		{
		if (NULL != (vsprite->CollMask =
			(WORD *)AllocMem(plane_size, MEMF_CHIP)))
			{
			vsprite->Y			= nVSprite->nvs_Y;
			vsprite->X			= nVSprite->nvs_X;
			vsprite->Flags		= nVSprite->nvs_Flags;
			vsprite->Width		= nVSprite->nvs_WordWidth;
			vsprite->Depth		= nVSprite->nvs_ImageDepth;
			vsprite->Height		= nVSprite->nvs_LineHeight;
			vsprite->MeMask		= nVSprite->nvs_MeMask;
			vsprite->HitMask	= nVSprite->nvs_HitMask;
			vsprite->ImageData	= nVSprite->nvs_Image;
			vsprite->SprColors	= nVSprite->nvs_ColorSet;
			vsprite->PlanePick	= 0x00;
			vsprite->PlaneOnOff	= 0x00;

			InitMasks(vsprite);
			return(vsprite);
			}
		FreeMem(vsprite->BorderLine, line_size);
		}
	FreeMem(vsprite, (LONG)sizeof(*vsprite));
	}
return(NULL);
}

/*-------------------------------------------------------------
** create a Bob from the information given in nBob.
** use freeBob() to free this gel.
**
** A VSprite is created for this bob.
** This routine properly allocates all double buffered information
** if it is required.
*/
struct Bob *makeBob(NEWBOB *nBob)
{
struct Bob		   *bob;
struct VSprite	   *vsprite;
NEWVSPRITE			nVSprite ;
LONG				rassize;

rassize = (LONG)sizeof(UWORD) *
			nBob->nb_WordWidth * nBob->nb_LineHeight * nBob->nb_RasDepth;

if (NULL != (bob =
	(struct Bob *)AllocMem((LONG)sizeof(struct Bob), MEMF_CLEAR)))
	{
	if (NULL != (bob->SaveBuffer = (WORD *)AllocMem(rassize, MEMF_CHIP)))
		{
		nVSprite.nvs_WordWidth	= nBob->nb_WordWidth;
		nVSprite.nvs_LineHeight	= nBob->nb_LineHeight;
		nVSprite.nvs_ImageDepth	= nBob->nb_ImageDepth;
		nVSprite.nvs_Image		= nBob->nb_Image;
		nVSprite.nvs_X			= nBob->nb_X;
		nVSprite.nvs_Y			= nBob->nb_Y;
		nVSprite.nvs_ColorSet	= NULL;
		nVSprite.nvs_Flags		= nBob->nb_BFlags;
		nVSprite.nvs_MeMask		= nBob->nb_MeMask;
		nVSprite.nvs_HitMask	= nBob->nb_HitMask;

		if ((vsprite = makeVSprite(&nVSprite)) != NULL)
			{
			vsprite->PlanePick = nBob->nb_PlanePick;
			vsprite->PlaneOnOff = nBob->nb_PlaneOnOff;

			vsprite->VSBob	 = bob;
			bob->BobVSprite	 = vsprite;
			bob->ImageShadow = vsprite->CollMask;
			bob->Flags		 = 0;
			bob->Before		 = NULL;
			bob->After		 = NULL;
			bob->BobComp	 = NULL;

			if (nBob->nb_DBuf)
				{
				if (NULL != (bob->DBuffer = (struct DBufPacket *)AllocMem(
					(LONG)sizeof(struct DBufPacket), MEMF_CLEAR)))
					{
					if (NULL != (bob->DBuffer->BufBuffer =
						(WORD *)AllocMem(rassize, MEMF_CHIP)))
						{
						return(bob);
						}
					FreeMem(bob->DBuffer, (LONG)sizeof(struct DBufPacket));
					}
				}
			else
				{
				bob->DBuffer = NULL;
				return(bob);
				}

			freeVSprite(vsprite);
			}
		FreeMem(bob->SaveBuffer, rassize);
		}
	FreeMem(bob, (LONG)sizeof(*bob));
	}
return(NULL);
}

/*-------------------------------------------------------------
** create a Animation Component from the information given in nAnimComp
** and nBob.
** use freeComp() to free this gel.
**
** makeComp calls makeBob(), and links the bob into a AnimComp.
*/
struct AnimComp *makeComp(NEWBOB *nBob, NEWANIMCOMP *nAnimComp)
{
struct Bob		*compBob;
struct AnimComp	*aComp;

if ((aComp = AllocMem((LONG)sizeof(struct AnimComp),MEMF_CLEAR)) != NULL)
	{
	if ((compBob = makeBob(nBob)) != NULL)
		{
		compBob->After	 = NULL;  /* Caller can deal with these later. */
		compBob->Before	 = NULL;
		compBob->BobComp = aComp;   /* Link 'em up. */

		aComp->AnimBob		= compBob;
		aComp->TimeSet		= nAnimComp->nac_Time; /* Num ticks active. */
		aComp->YTrans		= nAnimComp->nac_Yt; /* Offset rel to HeadOb */
		aComp->XTrans		= nAnimComp->nac_Xt;
		aComp->AnimCRoutine	= nAnimComp->nac_Routine;
		aComp->Flags		= nAnimComp->nac_CFlags;
		aComp->Timer		= 0;
		aComp->NextSeq		= NULL;
		aComp->PrevSeq		= NULL;
		aComp->NextComp		= NULL;
		aComp->PrevComp		= NULL;
		aComp->HeadOb		= NULL;

		return(aComp);
		}
	FreeMem(aComp, (LONG)sizeof(struct AnimComp));
	}
return(NULL);
}

/*-------------------------------------------------------------
** create an Animation Sequence from the information given in nAnimSeq
** and nBob.
** use freeSeq() to free this gel.
**
** this routine creates a linked list of animation components which
** make up the animation sequence.
**
** It links them all up, making a circular list of the PrevSeq
** and NextSeq pointers. That is to say, the first component of the
** sequences' PrevSeq points to the last component; the last component of
** the sequences' NextSeq points back to the first component.
**
** If dbuf is on, the underlying Bobs'll be set up for double buffering.
** If singleImage is non-zero, the pImages pointer is assumed to point to
** an array of only one image, instead of an array of 'count' images, and
** all Bobs will use the same image.
*/
struct AnimComp *makeSeq(NEWBOB *nBob, NEWANIMSEQ *nAnimSeq)
{
int seq;
struct AnimComp *firstCompInSeq = NULL;
struct AnimComp *seqComp = NULL;
struct AnimComp *lastCompMade = NULL;
LONG image_size;
NEWANIMCOMP	nAnimComp;

/* get the initial image.  this is the only image that is used
** if nAnimSeq->nas_SingleImage is non-zero.
*/
nBob->nb_Image = nAnimSeq->nas_Images;
image_size = nBob->nb_LineHeight * nBob->nb_ImageDepth * nBob->nb_WordWidth;

/* for each comp in the sequence */
for (seq = 0; seq < nAnimSeq->nas_Count; seq++)
	{
	nAnimComp.nac_Xt		= *(nAnimSeq->nas_Xt + seq);
	nAnimComp.nac_Yt		= *(nAnimSeq->nas_Yt + seq);
	nAnimComp.nac_Time		= *(nAnimSeq->nas_Times + seq);
	nAnimComp.nac_Routine	= nAnimSeq->nas_Routines[seq];
	nAnimComp.nac_CFlags	= nAnimSeq->nas_CFlags;

	if ((seqComp = makeComp(nBob, &nAnimComp)) == NULL)
		{
		if (firstCompInSeq != NULL)
			freeSeq(firstCompInSeq, (LONG)nBob->nb_RasDepth);
		return(NULL);
		}

	seqComp->HeadOb = nAnimSeq->nas_HeadOb;

	/* Make a note of where the first component is. */
	if (firstCompInSeq == NULL)
		firstCompInSeq = seqComp;

	/* link the component into the list */
	if (lastCompMade != NULL)
		lastCompMade->NextSeq = seqComp;

	seqComp->NextSeq = NULL;
	seqComp->PrevSeq = lastCompMade;
	lastCompMade = seqComp;

	/* If nAnimSeq->nas_SingleImage is zero,
	** the image array has nAnimSeq->nas_Count images.
	*/
	if (!nAnimSeq->nas_SingleImage)
		nBob->nb_Image += image_size;
	}
/* On The last component in the sequence, set Next/Prev to make
** the linked list a loop of components.
*/
lastCompMade->NextSeq = firstCompInSeq;
firstCompInSeq->PrevSeq = lastCompMade;

return(firstCompInSeq);
}

/*-------------------------------------------------------------
** free the data created by makeVSprite()
**
** assumes images deallocated elsewhere.
*/
VOID freeVSprite(struct VSprite *vsprite)
{
LONG	line_size;
LONG	plane_size;

line_size = (LONG)sizeof(WORD) * vsprite->Width;
plane_size = line_size * vsprite->Height;

FreeMem(vsprite->BorderLine, line_size);
FreeMem(vsprite->CollMask, plane_size);

FreeMem(vsprite, (LONG)sizeof(*vsprite));
}

/*-------------------------------------------------------------
** free the data created by makeBob()
**
** it's important that rasdepth match the depth you
** passed to makeBob() when this gel was made.
** assumes images deallocated elsewhere.
*/
VOID freeBob(struct Bob *bob, LONG rasdepth)
{
LONG	rassize;

rassize =  (LONG)sizeof(UWORD) *
		bob->BobVSprite->Width * bob->BobVSprite->Height * rasdepth;

if (bob->DBuffer != NULL)
	{
	FreeMem(bob->DBuffer->BufBuffer, rassize);
	FreeMem(bob->DBuffer, (LONG)sizeof(struct DBufPacket));
	}
FreeMem(bob->SaveBuffer, rassize);
freeVSprite(bob->BobVSprite);
FreeMem(bob, (LONG)sizeof(*bob));
}

/*-------------------------------------------------------------
** free the data created by makeComp()
**
** it's important that rasdepth match the depth you
** passed to makeComp() when this gel was made.
** assumes images deallocated elsewhere.
*/
VOID freeComp(struct AnimComp *myComp, LONG rasdepth)
{
freeBob(myComp->AnimBob, rasdepth);
FreeMem(myComp, (LONG)sizeof(struct AnimComp));
}

/*-------------------------------------------------------------
** free the data created by makeSeq()
**
** Complimentary to makeSeq(), this routine goes through the NextSeq
** pointers and frees the Components
**
** This routine only goes forward through the list, and so
** it must be passed the first component in the sequence, or the sequence
** must be circular (which is guaranteed if you use makeSeq()).
**
** it's important that rasdepth match the depth you
** passed to makeSeq() when this gel was made.
** assumes images deallocated elsewhere.
*/
VOID freeSeq(struct AnimComp *headComp, LONG rasdepth)
{
struct AnimComp *curComp;
struct AnimComp *nextComp;

/* this is freeing a loop of AnimComps, hooked together by the
** NextSeq and PrevSeq pointers.
*/

/* break the NextSeq loop, so we get a NULL at the end of the list. */
headComp->PrevSeq->NextSeq = NULL;

curComp = headComp;			/* get the start of the list */
while (curComp != NULL)
	{
	nextComp = curComp->NextSeq;
	freeComp(curComp, rasdepth);
	curComp = nextComp;
	}
}

/*-------------------------------------------------------------
** free an animation object (list of sequences).
**
** freeOb() goes through the NextComp pointers, starting at the AnimObs'
** HeadComp, and frees every sequence.
** it only goes forward. It then frees the Object itself.
** assumes images deallocated elsewhere.
*/
VOID freeOb(struct AnimOb *headOb, LONG rasdepth)
{
struct AnimComp *curSeq;
struct AnimComp *nextSeq;

curSeq = headOb->HeadComp;			/* get the start of the list */
while (curSeq != NULL)
	{
	nextSeq = curSeq->NextComp;
	freeSeq(curSeq, rasdepth);
	curSeq = nextSeq;
	}

FreeMem(headOb, (LONG)sizeof(struct AnimOb));
}
