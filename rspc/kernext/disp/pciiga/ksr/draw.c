static char sccsid[] = "@(#)52	1.1  src/rspc/kernext/disp/pciiga/ksr/draw.c, pciiga, rspc411, 9436D411b 9/5/94 16:30:43";
/*
 *
 * COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: draw_char, clear_lines, draw_text
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "iga_INCLUDES.h"
#include "igamisc.h"
#include "iga_funct.h"
#include "IO_defs.h"

BUGXDEF (dbg_drawch);
BUGXDEF (dbg_drawtxt);
BUGXDEF (dbg_clrlines);
BUGXDEF (dbg_colex);

/*-----------------------------------------------------------------------
 *
 * DESCRIPTION OF MODULE: 8 bit per pel color expander
 *
 * IDENTIFICATION :   colex
 *
 * DESCRIPTIVE NAME:  color expand and output to characters to IGA VRAM
 *
 *
 * FUNCTION:          This routine takes 1 bit per pel data and expands
 *                      it to ld->bpp (current bit per pel format)
 *                      It will do this until incrementing the x position
 *                      only (ie. only works on the current stride of the
 *                      character).
 *                    A background color (ld->bgcol) is used to expand 0's
 *                    A foreground color (ld->fgcol) is used to expand 1's
 *                      KSR mode only supports 16 colors
 * INPUTS:
 * OUTPUTS:           segbb8 (the ld->bpp expanded data)
 *
 * CALLED BY:         draw_char()
 *
 * CALLS:             NONE
 *
 *
 ----------------------------------------------------------------------*/

#define LITTLE_FB

#ifdef  LITTLE_FB
#define STORE_SHORT( data, addr )       *(addr) = (data)
#define STORE_LONG(  data, addr )       *(addr) = (data)
#else
#define STORE_SHORT( data, addr )       st_2r((data), (addr))
#define STORE_LONG(  data, addr )       st_4r((data), (addr))
#endif /* LITTLE_FB */

#define COLEX(num_pels,segbpp1,cur_x,cur_y,ld)    /* inline */          \
{                                                                       \
        int nbr_pels;                                                   \
        ushort preps;   /* number pels to achieve alignment  */         \
        ushort fullwps; /* number of 32 bit words per stride */         \
        ushort rempps;  /* remaining pels per stride  */                \
        ulong segbpp8;  /* font data in 8 bit per pel format */         \
        int jjj,kkk;                                                    \
        volatile ulong  *templ;                                         \
        volatile ushort *temps;                                         \
                                                                        \
        nbr_pels = num_pels;                                            \
                                                                        \
        /*----------------------------------------------------------    \
        | color expand the character by converting 32 bits in           \
        | 1 bit/pel (segbpp1) to 8*32 bits in 8 bits/pel                \
        | 8*(segbpp8) , then output the data in up to 32 bits of        \
        | 8BPP formatted data                                           \
        |---------------------------------------------------------*/    \
                                                                        \
        /*--------------------------------------------------------------\
         | achieve alignment, some systems can't handle unaligned stores\
         -------------------------------------------------------------*/\
        preps = cur_x & 3;                                              \
        nbr_pels -= preps;                                              \
                                                                        \
        /*---------------------------------------------                 \
         | get # of words that can be written                           \
         ---------------------------------------------*/                \
        fullwps=nbr_pels/(WORDLEN/BitsPP8);                             \
                                                                        \
        /*----------------------------------------                      \
         | get remaining # of pels that need to be written              \
         |----------------------------------------*/                    \
        rempps=nbr_pels%(WORDLEN/BitsPP8);                              \
                                                                        \
        BUGLPR(dbg_colex,BUGGID,                                        \
                ("CLX:preps=%d, nbr_pels=%d, fullwps=%d, rempps=%d\n",  \
                      preps,    nbr_pels,    fullwps,    rempps    ));  \
        /*--------------------------------------------------            \
         | write initial pixels (up to 3)                               \
         |------------------------------------------------*/            \
                                                                        \
        if(preps & 1)                                                   \
        {                                                               \
        /*--------------------------------------------------            \
         | do one pixel                                                 \
         |-------------------------------------------------*/           \
                segbpp8 <<=BitsPP8;                                     \
                segbpp8 += (segbpp1<0) ? ld->fgcol:ld->bgcol;           \
                segbpp1 <<= 1;                                          \
                                                                        \
                BUGLPR(dbg_colex,BUGGID,("CLX1a:HW(x%d, y%d)=seg%X\n",  \
                                           cur_x,cur_y,(uchar)segbpp8));\
                BUGLPR(dbg_colex,BUGGID,("(fg%X, bg%X)\n", ld->fgcol, ld->bgcol));\
                                                                        \
                /*------------------------------------------            \
                | write  8 bits of expanded data                        \
                |------------------------------------------*/           \
                                                                        \
                *FB_PIXEL(cur_x, cur_y) = (uchar)segbpp8;              \
                                                                        \
                cur_x++; /* move x pointer */                           \
                                                                        \
        } /* end if there odd number of initial pels to write */        \
                                                                        \
        if(preps > 1)                                                   \
        {                                                               \
                /*--------------------------------------------------    \
                 | do 2 pixels                                          \
                 |-------------------------------------------------*/   \
                segbpp8 <<=BitsPP8;                                     \
                segbpp8 += (segbpp1<0) ? ld->fgcol:ld->bgcol;           \
                segbpp1 <<= 1;                                          \
                segbpp8 <<=BitsPP8;                                     \
                segbpp8 += (segbpp1<0) ? ld->fgcol:ld->bgcol;           \
                segbpp1 <<= 1;                                          \
                                                                        \
                BUGLPR(dbg_colex,BUGGID,("CLX2a:HW(x%d, y%d)=seg%X\n",  \
                                          cur_x,cur_y,(ushort)segbpp8));\
                BUGLPR(dbg_colex,BUGGID,("(fg%X, bg%X)\n", ld->fgcol, ld->bgcol));\
                                                                        \
                /*------------------------------------------            \
                | write 16 bits of expanded data                        \
                |------------------------------------------*/           \
                                                                        \
                temps = (ushort)FB_PIXEL(cur_x, cur_y);                 \
                STORE_SHORT( segbpp8, temps);                           \
                                                                        \
                cur_x += 2; /* move x pointer */                        \
                                                                        \
        } /* end if there are > 1 initial pels to write */              \
                                                                        \
        /*----------------------------------------                      \
         | get remaining # of pels that need to be written              \
         |----------------------------------------*/                    \
        rempps=nbr_pels%(WORDLEN/BitsPP8);                              \
                                                                        \
        if(fullwps > 0)                                                 \
        {                                                               \
            /* write fullwps full words */                              \
            for (jjj=0; jjj<fullwps; jjj++ )                            \
            {                                                           \
                /* get 4 groups of 8bpp pels */                         \
                segbpp8 = 0;                                            \
                                                                        \
                /*create a full word of 8bpp data*/                     \
                                                                        \
                for (kkk=0; kkk<WORDLEN/BitsPP8 ;kkk++)                 \
                {                                                       \
                        /*----------------------------------------      \
                        | shift the postexpanded data 8bits             \
                        | and the pre expanded data 1 bit               \
                        |----------------------------------------*/     \
                        segbpp8 <<=BitsPP8;                             \
                        segbpp8 += (segbpp1<0) ? ld->fgcol:ld->bgcol;   \
                        segbpp1 <<= 1;                                  \
                }                                                       \
                                                                        \
                /*--------------------------------------------------    \
                | fullwps's are already left justified                  \
                |--------------------------------------------------*/   \
                                                                        \
                BUGLPR(dbg_colex,BUGGID,("CLX4:HW(x%d, y%d)=seg%X\n",   \
                                                  cur_x,cur_y,segbpp8));\
                BUGLPR(dbg_colex,BUGGID,("(fg%X, bg%X)\n", ld->fgcol, ld->bgcol));\
                                                                        \
                /*--------------------------------------------------    \
                | write 32 bits of expanded data                        \
                |--------------------------------------------------*/   \
                                                                        \
                templ = (ulong)FB_PIXEL(cur_x, cur_y);                  \
                STORE_LONG( segbpp8, templ);                            \
                                                                        \
                cur_x+=WORDLEN/BitsPP8; /* move x pointer */            \
            }                                                           \
        }                                                               \
                                                                        \
        /*--------------------------------------------------            \
         | write remaining pixels (up to 3)                             \
         |------------------------------------------------*/            \
                                                                        \
        if(rempps > 1)                                                  \
        {                                                               \
                /*--------------------------------------------------    \
                 | do 2 pixels                                          \
                 |-------------------------------------------------*/   \
                segbpp8 <<=BitsPP8;                                     \
                segbpp8 += (segbpp1<0) ? ld->fgcol:ld->bgcol;           \
                segbpp1 <<= 1;                                          \
                segbpp8 <<=BitsPP8;                                     \
                segbpp8 += (segbpp1<0) ? ld->fgcol:ld->bgcol;           \
                segbpp1 <<= 1;                                          \
                                                                        \
                BUGLPR(dbg_colex,BUGGID,("CLX2b:HW(x%d, y%d)=seg%X\n",  \
                                          cur_x,cur_y,(ushort)segbpp8));\
                BUGLPR(dbg_colex,BUGGID,("(fg%X, bg%X)\n", ld->fgcol, ld->bgcol));\
                                                                        \
                /*------------------------------------------            \
                | write 16 bits of expanded data                        \
                |------------------------------------------*/           \
                                                                        \
                temps = (ushort)FB_PIXEL(cur_x, cur_y);                 \
                STORE_SHORT( segbpp8, temps);                           \
                                                                        \
                cur_x += 2; /* move x pointer */                        \
                                                                        \
        } /* end if there are > 1 remaining pels to write */            \
        if(rempps & 1)                                                  \
        {                                                               \
        /*--------------------------------------------------            \
         | do 1 pixel                                                   \
         |-------------------------------------------------*/           \
                segbpp8 <<=BitsPP8;                                     \
                segbpp8 += (segbpp1<0) ? ld->fgcol:ld->bgcol;           \
                segbpp1 <<= 1;                                          \
                                                                        \
                BUGLPR(dbg_colex,BUGGID,("CLX1b:HW(x%d, y%d)=seg%X\n",  \
                                           cur_x,cur_y,(uchar)segbpp8));\
                BUGLPR(dbg_colex,BUGGID,("(fg%X, bg%X)\n", ld->fgcol, ld->bgcol));\
                                                                        \
                /*------------------------------------------            \
                | write  8 bits of expanded data                        \
                |------------------------------------------*/           \
                                                                        \
                *FB_PIXEL(cur_x, cur_y) = (uchar)segbpp8;              \
                                                                        \
                cur_x++; /* move x pointer */                           \
                                                                        \
        } /* end if there are remaining pels to write */                \
} /* end COLEX */

/*---------------------------------------------------------------------
 *
 * IDENTIFICATION: DRAW_CHAR
 *
 * DESCRIPTIVE NAME: Draw a character to the iga adapter.
 *
 * CALLED BY: Draw_text
 *
 * CALLS:     colex()
 *
 ---------------------------------------------------------------------*/
void
draw_char (vp, xo, yo, char_offset, cursor_char)
        struct vtmstruc *vp;
        ushort          xo,
                        yo,
                        char_offset;
        char            cursor_char;   /* nonzero if cursor is on the
                                        * character */

{
        /* All uses are longs...aixGlyphPtr charptr; */
        ulong           indexptr;      /* workarea pointer to character index  */
        ulong          *charptr;       /* workarea pointer to character
                                        * bitmap THIS IS A LONG */
        ulong          *cursorptr;     /* pointer to cursor bit map */
        ulong          *icharptr;      /* pointer to cursor inverted
                                        * character bit map */
        struct iga_data *ld;

        ushort          boxw,
                        boxh,
                        boxfw,
                        boxf;
        int             ht;
        ulong           segbpp8;
        long            segbpp1;       /* must be signed */
        int             cur_y,

                        cur_x;
        int             partseg,
                        fullseg,
                        wholseg;
        int             wth;
        int             jjj,
                        kkk;
        ulong           tmp;           /* for macro internal use... may be
                                        * replace by segbpp8 */
        char            index;
        struct iga_ddf  *ddf;
        ld = (struct iga_data *) vp->vttld;

        /* get to desired character's offset index */
        indexptr = (unsigned long) ld->fontcindex[char_offset].byteOffset;

BUGLPR (dbg_drawch, BUGACT, ("BASE INDEX POINTER ( index = 0 ) =0x%X\n", ld->fontcindex[0].byteOffset));
BUGLPR (dbg_drawch, BUGACT, ("char_offset=0x%X\n", char_offset));
BUGLPR (dbg_drawch, BUGACT, ("indexptr=0x%X\n", indexptr));

        /* get to desired character within the font map */
        /*
         * all access are longs....
         * charptr=(aixGlyphPtr)((ulong)ld->Font_map_start +

         * (ulong)indexptr);
         */

        charptr = (ulong *) ((ulong) ld->glyphptr + (ulong) indexptr);

        /* set up the font dimensions */
        boxh = ld->Vttenv.character_box.height; /* height of character to
                                                 * display */
        boxw = ld->Vttenv.character_box.width;  /* width  of character to
                                                 * display */
        boxfw = ld->Vttenv.font_box_width;      /* width of character plus
                                                 * font filler */
        boxf = boxfw - boxw;           /* font filler per stride */
        BUGLPR (dbg_drawch, BUGNTA, ("DCH: boxh=%d    boxw=%d\n",boxh, boxw));
        if (cursor_char)
        {
                icharptr = (ulong *) ld->Vttenv.cursor.ichar;
                cursorptr = (ulong *) ld->Vttenv.cursor.data;
                BUGLPR (dbg_drawch, BUGACT, ("cursorptr=0x%X\n", cursorptr));
                BUGLPR (dbg_drawch, BUGACT, ("icharptr=0x%X\n", icharptr));
                BUGLPR (dbg_drawch, BUGACT, ("charptr=0x%X\n", charptr));
                BUGLPR (dbg_drawch, BUGACT, ("malloc size=%X\n", ld->Vttenv.cursor.lastmalloc));

                if ((ht = boxh * boxfw) % WORDLEN)      /* assignment */
                {
                        ht = ht / WORDLEN + 1;
                }
                else
                {
                        ht = ht / WORDLEN;
                }

                for (index = 0; index < ht; index++)
                {                      /* for all bytes of cursor data */
                        /* make a copy of the character */
                        /* and XOR the bits with the cursor mask */

                        icharptr[index] = charptr[index] ^ cursorptr[index];
                }

                charptr = (ulong *) icharptr;   /* set charptr to cursor
                                                 * inverted character */
        }
        cur_y = (yo - 1) * boxh;       /* set current y pel to the top pel of
                                        * the row */
        cur_x = (xo - 1) * boxw;       /* set current x pel to the 1st pel of
                                        * the col */
        partseg = 0;
        ht = boxh;

	ddf = ld->ddf;			/* set addressability for attach */
        ld->bus_base_addr = PCI_BUS_ATT(&ddf->pci_mem_map);

        while (ht > 0)
        {
                /*-------------------------------------------------------
                 *  a data segment (segbpp1) refers to the 1 bit/pel
                 *    data that is read from glyphprt       in 32 bit
                 *    intervals
                 *  a stride refers to the character box width in pels
                 *  NOTE: in seg1bpp, 1 bit refers to 1 pel
                 *  NOTE: in seg8bpp, 8 bits refer to 1 pel
                 *  fullseg=# of full segments in a stride (bits/bits)
                 *  partseg accounts for portion done by leftover
                 *    bits in the previous segment's data
                -------------------------------------------------------*/

                fullseg = (boxw - partseg) / (WORDLEN);
                partseg = (boxw - partseg) % (WORDLEN); /* bits leftover */

                if (boxw >= WORDLEN)
                {
                        BUGLPR (dbg_drawch, BUGACT, ("BOXW >= WORDLEN\n"));
                        /*-------------------------------------------------------
                         * Here, we can read less bits in a 32 bit read
                         * than it will take to  fill a stride
                         * fit a number of 1 bpp segments into this stride
                        -------------------------------------------------------*/
                        for (wth = 0; wth < fullseg; wth++)
                        {
                                /* if in here, full word won't fill stride */
                                /* (i.e.   stride is over 4 bytes long)  */

                                BUGLPR (dbg_drawch, BUGACT, ("chptr=%X  = *=%X\n", charptr, *charptr));
                                segbpp1 = (int) *charptr++;     /* get a data segment */
                                if (ld->reverse_video)
                                        segbpp1 ^= 0xFFFFFFFF;
                                /*
                                 * and color expand that data segent and
                                 * write it to IGA VRAM
                                 */
                                BUGLPR (dbg_drawch, BUGNTA, ("pre colex\n"));
                                COLEX (WORDLEN, segbpp1, cur_x, cur_y, ld);
                                BUGLPR (dbg_drawch, BUGNTA, ("postcolex\n"));
                        }
                        /*-------------------------------------------------------
                         * stride may not be complete at this point
                         * but the current segment has been fully used
                         * Partseg contains the number of bits needed
                         * to complete the stride
                        -------------------------------------------------------*/
                        if (partseg)
                        {

                                segbpp1 = *charptr++;   /* get a data segment */
                                if (ld->reverse_video)
                                        segbpp1 ^= 0xFFFFFFFF;

                                BUGLPR (dbg_drawch, BUGNTA, ("In Partseg,precolex\n"));
                                COLEX (partseg, segbpp1, cur_x, cur_y, ld);

                                BUGLPR (dbg_drawch, BUGNTA, ("post colex\n"));
                                partseg = WORDLEN - partseg;
                        }
                        BUGLPR (dbg_drawch, BUGNTA, ("outof partseg\n"));
                        cur_y++;       /* increment the y position counter by
                                        * pels(notCHAR) */
                        cur_x = (xo - 1) * boxw;        /* reset current xpel to
                                                         * the 1st pel of the
                                                         * col */
                        ht--;          /* decrement the height counter */
                        if (ht <= 0)
                        {
                                break;
                        }
                        /*-------------------------------------------------------
                         * Stride is fully complete at this point
                         * but the segment may only be partially used
                         * and/or the segment may contain useless filler
                         * Test to see if there is filler and get rid of it.
                        -------------------------------------------------------*/
                        if (boxf)
                        {
                                segbpp1 <<= boxf;
                                partseg -= boxf;
                        }              /* NOTE: filler fills to end of 32 bit
                                        * word.So, partseg cannot be neg */
                        /*-------------------------------------------------------
                         * Stride is fully complete at this point
                         * but the segment may only be partially used
                         * Most significant bits in seg are for the next stride
                         * and partseg contains the # of bits left
                        -------------------------------------------------------*/
                }
                else
                {
                        BUGLPR (dbg_drawch, BUGACT, ("BOXW < WORDLEN\n"));
                        /*-----------------------------------------------
                         * make this segement into a number of strides
                         * NOTE: Each stride may or may not have font
                         * filler at the end that needs to be discarded
                        -----------------------------------------------*/

                        if (partseg)
                        {
                                segbpp1 = *charptr++;
                                if (ld->reverse_video)
                                        segbpp1 ^= 0xFFFFFFFF;
                                wholseg = WORDLEN;
                                while((partseg + boxf <= wholseg) &&    /* while we can expand*/
                                      (partseg + boxf > 0 )        )    /* full strides */
                                {
                                        /*
                                         * only expand the stride length,
                                         * boxw. partseg may contain filler
                                         */
                                        COLEX (partseg, segbpp1, cur_x, cur_y, ld);
                                        /*
                                         * now at the end of a stride, get
                                         * rid of filler if there is any
                                         */
                                        if (boxf)
                                        {
                                                segbpp1 <<= boxf;
                                        }
                                        /*
                                         * determine remaining pels by
                                         * subtracting stride length and
                                         * filler
                                         */
                                        wholseg -= partseg + boxf;

                                        cur_y++;
                                        cur_x = (xo - 1) * boxw;
                                        ht--;
                                        /*
                                         * partseg needs to be reset for the
                                         * full stride
                                         */
                                        partseg = (boxw) % (WORDLEN);
                                        if (ht <= 0)
                                        {
                                                break;
                                        }
                                }
                                partseg = wholseg;
                        }
                        /*-------------------------------------------------------
                         * Stride is fully complete at this point
                         * but the segment may only be partially used
                         * Most significant bits in seg are for the next stride
                         * and partseg contains the # of bits left
                        -------------------------------------------------------*/
                }
                if (ht <= 0)
                {
                        break;
                }

                /* now finish off rest of bits in segment on the next stride */
                if (partseg && (cur_y <= (yo - 1) * boxh + boxh))
                {
                        /* expand remaining pels of this segment */
                        /*
                         * NOTE: returned segbpp1 will be empty of valid
                         * information
                         */
                        COLEX (partseg, segbpp1, cur_x, cur_y, ld);

                        /*-------------------------------------------------------
                         * stride may not be complete at this point
                         * but the current segment has been fully used.
                         * Partseg contains the number of bits used so far
                         * to complete the stride
                        -------------------------------------------------------*/
                }
        }
        BUGLPR (dbg_drawch, BUGNTA, ("CHAR DONE (ch%X cur%d x%d y%d\n", char_offset, cursor_char, xo, yo));
        PCI_BUS_DET(ld->bus_base_addr);
}


/*-------------------------------------------------------------------------
 *
 * DESCRIPTION OF MODULE:
 *
 * IDENTIFICATION :   clear_lines
 *
 * DESCRIPTIVE NAME:
 *
 *
 * FUNCTION:
 *
 * INPUTS:
 *
 * OUTPUTS:
 *
 * CALLED BY:
 *
 * CALLS:
 *
 *
-------------------------------------------------------------------------*/
clear_lines (vp, ld, topline, botline, linelen, xstart)
        struct vtmstruc *vp;
        struct iga_data *ld;
        unsigned short  topline,
                        botline;
        unsigned short  linelen,
                        xstart;
{
        unsigned long   bgword;
        unsigned char   advance;
        unsigned short  pre_pels;
        unsigned short  fullwpl;
        unsigned short  remppl;
        unsigned short  xpos;
        short           tmp;
        volatile ulong  *templ;
        volatile ushort *temps;
        struct iga_ddf   *ddf;
        /* Put in fast clear routine */
        /* make sure code around "border" */
        BUGLPR (dbg_clrlines, BUGNTA, (">ClrLines, linelen=%d\n",linelen));
        BUGLPR (dbg_clrlines, BUGGID, ("ld->bpp =%d\n",ld->bpp));
        pre_pels = xstart & 3;
        tmp = WORDLEN / ld->bpp;
        /* get the number of full words / line  */
        fullwpl = (linelen - pre_pels) / tmp;
        /* get the remaining pels / line  */
        remppl = (linelen - pre_pels) % tmp;
        /* full word advances xposition by 32(1bpp) or 8(4bpp) pels */
        advance = tmp;
        BUGLPR (dbg_clrlines, BUGGID, ("pre_pels=%d\n",pre_pels));
        BUGLPR (dbg_clrlines, BUGGID, ("fullwpl =%d\n",fullwpl ));
        BUGLPR (dbg_clrlines, BUGGID, ("remppl  =%d\n",remppl  ));
        BUGLPR (dbg_clrlines, BUGGID, ("tmp     =%d\n",tmp     ));

        /* fill a full word with the background color */
        /* initialize background word with bg color */
        /* for 1 and 4 bit per pel, fill remaining 28 bits worth */

        bgword = (unsigned long) ld->bgcol * 0x01010101L;

        /* begin clearing lines */

        BUGLPR (dbg_clrlines, BUGNTA, ("attaching PCI Bus\n"));

	ddf = ld->ddf;		/* set addressability for attach */
        ld->bus_base_addr = PCI_BUS_ATT(&ddf->pci_mem_map);

        linelen = xstart + fullwpl * (WORDLEN / ld->bpp);

        BUGLPR (dbg_clrlines, BUGNFO, ("linelen =0x%8.8x\n",linelen));
        for (; topline < botline; topline++)
        {
        /*-------------------------------------------------------------
         | check alignment, some systems can't handle unaligned stores
         -------------------------------------------------------------*/
                xpos = xstart;

                if(pre_pels & 1)
                {
                        /*------------------------------------------
                        | store  8 bits
                        |------------------------------------------*/

                        *FB_PIXEL(xpos, topline)=(uchar)bgword;
                        xpos += 1;

                } /* end if there are odd No. initial pels to write */

                if(pre_pels > 1)
                {
                        /*------------------------------------------
                        | store 16 bits
                        |------------------------------------------*/

                        temps = (ushort)FB_PIXEL(xpos, topline);
                        *temps = (ushort)bgword;
                        xpos += 2;

                } /* end if there are > 1 initial pels to write */

                for (; xpos < linelen; xpos+=advance)
                {
                        /*------------------------------------------
                        | store 32 bits
                        |------------------------------------------*/

                        templ = (ulong)FB_PIXEL(xpos, topline);
        BUGLPR (dbg_clrlines, BUGNFO, ("templ =0x%x bgword =0x%x\n",templ, bgword));
                        *templ = bgword;
                }
                if(remppl > 1)
                {
                        /*------------------------------------------
                        | store 16 bits
                        |------------------------------------------*/

                        temps = (ushort)FB_PIXEL(xpos, topline);
                        *temps = (ushort)bgword;

                        xpos += 2; /* move x pointer */

                } /* end if there are > 1 remaining pels to write */
                if(remppl & 1)
                {
                        /*------------------------------------------
                        | store  8 bits
                        |------------------------------------------*/

                        *FB_PIXEL(xpos, topline)=(uchar)bgword;

                } /* end if there are odd No. remaining pels to write */
        }
        BUGLPR (dbg_clrlines, BUGNTA, ("detaching PCI Bus\n"));
        PCI_BUS_DET(ld->bus_base_addr);
}
/*-------------------------------------------------------------------------
 *
 * DESCRIPTION OF MODULE:
 *
 * IDENTIFICATION :   draw_text
 *
 * DESCRIPTIVE NAME:  Draw text to entry  adapter.
 *
 *
 * FUNCTION:  Loops through passed in string searching for substrings
 *              with identical attributes. Draws characters on at a time
 *              with calls to draw_char.
 *
 * INPUTS:   Virtual terminal pointer
 *             Pointer to string
 *             String length
 *             X and Y position to draw char
 *             Flag for whether attributes are constant
 *             Flag indicating whether ok to skip draw of blanks
 *
 * OUTPUTS:   NONE.
 *
 * CALLED BY: copy_ps()
 *              vtttext()
 *
 * CALLS:  draw_char
 *
 *
-------------------------------------------------------------------------*/
void
draw_text (vp, string, string_length, x, y,
           constant_attributes, draw_blanks)

        struct vtmstruc *vp;
        ulong           string[];
ushort          string_length,
                x,
                y;
long            constant_attributes;
char            draw_blanks;
{
        union
        {
                unsigned long   tmp_char;
                struct
                {
                        unsigned short  hi;
                        unsigned short  lo;
                }               tmp_char_;
        }               union_char;
        ulong           tmp;
        long            j;
        long            characters_written;
        long            attributes_change;
        ushort          current_character,
                        cur_x,
                        cur_y;
        short           characters_to_draw;
        struct iga_data *ld;
        uchar           first_blank;
        BUGLPR (dbg_drawtxt, BUGNTA, ("In DRAWTEXT(%X%X%X%X,len=%d,x=%d,y=%d,DB=%d)\n",
                                      string[0], string[1], string[2], string[3], string_length, x, y, draw_blanks));

        ld = (struct iga_data *) vp->vttld;

        cur_x = x;
        cur_y = y;

        if (ld->bgcol != ld->old_bgcol)
                /*
                 * if space bgcol is different from existing bgcol force
                 * blank drawing
                 */
        {
                draw_blanks = TRUE;
        }

        if (constant_attributes)
        {
                BUGLPR (dbg_drawtxt, BUGNTA, ("Handling constant attributes.\n"));
                current_character = 0;
                union_char.tmp_char = string[current_character];
                /* current_attr is originally set in vttinit to 0xFFFF */
                if (ld->Vttenv.current_attr != union_char.tmp_char_.lo)
                {
                        /*-----------------------------------------
                         * set the new character attributes......
                          -----------------------------------------*/
                        SET_CHAR_ATTRIB (ld, string[current_character], tmp);
                }

                /* Now that attributes are established send string out */
                first_blank = 1;

                BUGLPR (dbg_drawtxt, BUGNTA, ("Sending out string.\n"));
                for (j = current_character; j < string_length; j++)
                {
                        /*---------------------------------------------------------------
                         * we don't have to draw blanks do to below clear_lines() call
                         * so, only if char is not a blank, draw the char
                        ---------------------------------------------------------------*/
                        union_char.tmp_char = string[j];
                        if (union_char.tmp_char_.hi != 0x20)
                        {
                                draw_char (vp, cur_x, cur_y, union_char.tmp_char_.hi, NOCURSOR);
                        }
                        else
                        if (first_blank)
                        {
                                /*
                                 * Only clear lines on 1st blank to avoid
                                 * redrawing earlier text
                                 */
                                /*
                                 * fast clear entire area to avoid drawing
                                 * blanks
                                 */
                BUGLPR (dbg_drawtxt, BUGNTA, ("PRE Clear lines.\n"));
                                clear_lines (vp, ld,
                                             (cur_y - 1) * ld->Vttenv.character_box.height,
                                  (cur_y) * ld->Vttenv.character_box.height,
                                             (string_length - j) * ld->Vttenv.character_box.width,
                                             (cur_x - 1) * ld->Vttenv.character_box.width);
                BUGLPR (dbg_drawtxt, BUGNTA, ("POST Clear lines.\n"));
                                first_blank = 0;
                        }
                        cur_x++;
                }
        }
        else                           /* if not constant attributes */
        {
                BUGLPR (dbg_drawtxt, BUGNTA, ("Not constant attributes.\n"));
                /* This code loops until it finds a character with changed     */
                /*
                 * attributes. When it finds one it writes out all non
                 * changed
                 */
                /* characters then changes the attributes on the requested     */
                /* character resets the change flag and continues.               */

                attributes_change = FALSE;
                characters_written = 0;
                do
                {                      /* non-constant attributes */

                        for (current_character = characters_written;
                             current_character < string_length;
                             current_character++)
                        {
                                union_char.tmp_char = string[current_character];
                                if (union_char.tmp_char_.lo != ld->Vttenv.current_attr)
                                {
                                        attributes_change = TRUE;
                                        break;
                                }
                        }

                        characters_to_draw = current_character - characters_written;
                        if (characters_to_draw > 0)
                        {
BUGLPR (dbg_drawtxt, BUGGID, ("characters_to_draw             =%d\n",characters_to_draw));
                                /* write string of characters to adapter */
                                if (draw_blanks)
                                {
                                        /*
                                         * eliminate need to draw blanks by
                                         * clearing chars_to_draw to bgcolor
                                         */
BUGLPR (dbg_drawtxt, BUGNTA, ("PRE Clear lines.\n"));
BUGLPR (dbg_drawtxt, BUGGID, ("ld->Vttenv.character_box.height=%d\n",ld->Vttenv.character_box.height));
BUGLPR (dbg_drawtxt, BUGGID, ("ld->Vttenv.character_box.width =%d\n",ld->Vttenv.character_box.width));
                                        clear_lines (vp, ld,
                                            (cur_y - 1) * ld->Vttenv.character_box.height,
                                            (cur_y) * ld->Vttenv.character_box.height,
                                            characters_to_draw * ld->Vttenv.character_box.width,
                                            (cur_x - 1) * ld->Vttenv.character_box.width);
BUGLPR (dbg_drawtxt, BUGNTA, ("POST Clear lines.\n"));
                                }
                                /*--------------------------------------------------------------------
                                 * draw characters but DO NOT draw blanks
                                 *
                                 * if the draw_blanks flag is on, we have cleared the segment of
                                 *   characters_to_draw characters to the background color so there
                                 *   is no longer a need to draw blanks
                                 * if the draw_blanks flag is off we don't wish to draw them anyway
                                --------------------------------------------------------------------*/
                                for (j = characters_written; j < current_character; j++)
                                {
                                        union_char.tmp_char = string[j];
                                        if (union_char.tmp_char_.hi != 0x20)    /* not space,have to
                                                                                 * draw */
                                        {
                                                draw_char (vp, cur_x, cur_y, union_char.tmp_char_.hi, NOCURSOR);
                                        }
                                        cur_x++;
                                }
                        }

                        if (attributes_change && current_character < string_length)
                        {
                                /*-----------------------------------------
                                 * set the new character attributes......
                                -----------------------------------------*/
                                SET_CHAR_ATTRIB (ld, string[current_character], tmp);
                                attributes_change = FALSE;
                        }
                        characters_written = current_character;
                } while (characters_written < string_length);
        }                              /* End of the else attributes change
                                        * block */
        BUGLPR (dbg_drawtxt, BUGNTA, ("Exit DRAWTEXT\n"));
}                                      /* end of draw text */
