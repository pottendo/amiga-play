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

#ifndef GELTOOLS_H
#define GELTOOLS_H

/* these data structures are used by the functions in animtools.c to
** allow for an easier interface to the animation system.
*/

/* data structure to hold information for a new vsprite.
** note that:
**     NEWVSPRITE myNVS;
** is equivalent to:
**     struct newVSprite myNVS;
*/
typedef struct newVSprite
	{
	WORD		   *nvs_Image;		/* image data for the vsprite	*/
	WORD		   *nvs_ColorSet;	/* color array for the vsprite	*/
	SHORT			nvs_WordWidth;	/* width in words				*/
	SHORT			nvs_LineHeight;	/* height in lines				*/
	SHORT			nvs_ImageDepth;	/* depth of the image			*/
	SHORT			nvs_X;			/* initial x position			*/
	SHORT			nvs_Y;			/* initial y position			*/
	SHORT			nvs_Flags;		/* vsprite flags				*/
	USHORT			nvs_HitMask;	/* Hit mask.					*/
	USHORT			nvs_MeMask;		/* Me mask.						*/
	} NEWVSPRITE;

/* data structure to hold information for a new bob.
** note that:
**     NEWBOB myNBob;
** is equivalent to:
**     struct newBob myNBob;
*/
typedef struct newBob
	{
	WORD	   *nb_Image;		/* image data for the bob		*/
	SHORT		nb_WordWidth;	/* width in words				*/
	SHORT		nb_LineHeight;	/* height in lines				*/
	SHORT		nb_ImageDepth;	/* depth of the image			*/
	SHORT		nb_PlanePick;	/* planes that get image data	*/
	SHORT		nb_PlaneOnOff;	/* unused planes to turn on		*/
	SHORT		nb_BFlags;		/* bob flags					*/
	SHORT		nb_DBuf;		/* 1=double buf, 0=not			*/
	SHORT		nb_RasDepth;	/* depth of the raster			*/
	SHORT		nb_X;			/* initial x position			*/
	SHORT		nb_Y;			/* initial y position			*/
	USHORT		nb_HitMask;		/* Hit mask.					*/
	USHORT		nb_MeMask;		/* Me mask.						*/
	} NEWBOB ;

/* data structure to hold information for a new animation component.
** note that:
**     NEWANIMCOMP myNAC;
** is equivalent to:
**     struct newAnimComp myNAC;
*/
typedef struct newAnimComp
	{
	WORD  (*nac_Routine)();	/* routine called when Comp is displayed.	*/
	SHORT	nac_Xt;			/* initial delta offset position.			*/
	SHORT	nac_Yt;			/* initial delta offset position.			*/
	SHORT	nac_Time;		/* Initial Timer value.						*/
	SHORT	nac_CFlags;		/* Flags for the Component.					*/
	} NEWANIMCOMP;

/* data structure to hold information for a new animation sequence.
** note that:
**     NEWANIMSEQ myNAS;
** is equivalent to:
**     struct newAnimSeq myNAS;
*/
typedef struct newAnimSeq
	{
	struct AnimOb  *nas_HeadOb;	/* common Head of Object.				*/
	WORD   *nas_Images;			/* array of Comp image data				*/
	SHORT  *nas_Xt;				/* arrays of initial offsets.			*/
	SHORT  *nas_Yt;				/* arrays of initial offsets.			*/
	SHORT  *nas_Times;			/* array of Initial Timer value.		*/
	WORD (**nas_Routines)();	/* Array of fns called when comp drawn	*/
	SHORT	nas_CFlags;			/* Flags for the Component.				*/
	SHORT	nas_Count;			/* Num Comps in seq (= arrays size) 	*/
	SHORT	nas_SingleImage;	/* one (or count) images.				*/
	} NEWANIMSEQ;

#endif
