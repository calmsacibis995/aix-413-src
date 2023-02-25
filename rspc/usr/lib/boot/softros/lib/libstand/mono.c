static char sccsid[] = "@(#)17	1.1  src/rspc/usr/lib/boot/softros/lib/libstand/mono.c, rspc_softros, rspc411, GOLD410 4/18/94 15:49:41";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: display_putc
 *		init_mono
 *		scroll_up
 *		set_cursor
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Memory mapped display adapters with emphasis on mono                      */
#include <stdio.h>
#include "mono.h"

struct mono monodata;            /* Global monochrome adapter data     */
unsigned long monoerrs = 0;      /* Count of errors found              */
static unsigned char init_data[] = {0x61, 0x50, 0x52, 0x0f, 0x19, 0x06, 0x19,
                                    0x19, 0x02, 0x0d, 0x0c, 0x0d,
                                    0x00, 0x00, 0x00, 0x00, 0x00};
static int fcntmono = 0;           /* Holds current screen position          */
static int initmono = 0;           /* Knows if MONO has been initialized     */
static int fcntvga = 0;            /* Holds current VGA screen position      */
static int initvga = 0;	           /* Knows if VGA has been initialized      */
static int initfga = 0;        	   /* Knows if FGA has been initialized      */

#define LINE_LENGTH             80
#define LINES_PER_SCREEN        25
#define TOTAL_SCREEN_BYTES      4000
#define BEG_2ND_LINE            160
#define BEG_LAST_LINE           3840



/* set_cursor - This routine just puts the cursor to where the next    */
/*              character will be displayed                            */


static void set_cursor(int port, unsigned short val)

{
  int high, low;
  int index, data;

  switch (port)
    {
    case 1:                        /* Mono display                           */
      index = MONO_INDEX;
      data = MONO_DATA;
      break;

    case 2:                        /* VGA display                            */
      index = 0x800003d4;          /* Color control register + SF I/O offset */
      data = index+1;
      break;

    default:                       /* Anything else                          */
      return;
    }

  high = (val >> 8) & 0xff;
  low = val & 0xff;

  HW_WRRB(index, 14);              /* 14 is cursor high                      */
  HW_WRRB(data,high);

  HW_WRRB(index, 15);              /* 15 is cursor low                       */
  HW_WRRB(data,low);

  return;
};



/* init_mono - This routine will attempt to initialize the monochrome  */
/*             display adapter.  If it can't find one, or it can't     */
/*             initiailze it for some reason, it will return 0.  If    */
/*             everything worked it will return 1.                     */
int init_mono()

{
  int i;                           /* Loop variable do wo write              */
  int len;                         /* Length of structure init_data          */

  /* Don't get crazy about this initialization stuff                         */
  if (initmono)                    /* Don't do it again                      */
    return;

  /* Step 1 - Tell control port that we are going into hires mode            */
  HW_WRRB(MONO_CNTRL,MODE_HIRES);

  /* Step 2 - Write the initial register contents to all internal registers  */
  for (i=0; i<sizeof(init_data); i++)
    {
    HW_WRRB(MONO_INDEX, i);
    HW_WRRB(MONO_DATA, init_data[i]);
    };

  /* Step 3 - Enable video and set correct port value (Stolen from PC BIOS)  */
  HW_WRRB(MONO_CNTRL, MODE_SET);

  /* Step 4 - Set overscan register from BIOS (Probably not necessary)       */
  HW_WRRB(MONO_CNTRL+1, 0x30);

  /* Step 5 - Initialize the global monodata structure                       */
  monodata.rows    = MROWS;        /* Total number of rows                   */
  monodata.cols    = MCOLS;
  monodata.currow  = 0;            /* Upper left corner of the display       */
  monodata.curcol  = 0;
  monodata.membase = (unsigned char *)MONO_MEM;
  monodata.cursor  = 0x0f0e;       /* Default cursor shape                   */

  /* Step 6 - Clear the display memory                                       */
  for (i=0;i<2000;i++)
    {
    outbmem(MONO_BASE+2*i,' ');
    outbmem(MONO_BASE+2*i+1, 0x07);
    }
}


int
display_putc(int port, char c)
{

    int print_char = 0;
    int fcnt;                      /* Holds current screen position          */
    unsigned int base;             /* Base address of display buffer         */

    /* Setup the port specific stuff                                         */
    switch (port)
      {
      case 1:                      /* Mono display adapter                   */
        if (initmono == 0)         /* Initialize if not yet done             */
          {
          init_mono();
          initmono = 1;
          }
        fcnt = fcntmono;           /* Get current mono screen position offset*/
        base = MONO_BASE;          /* Base address of adapter                */
        break;

      case 2:                      /* VGA                                    */
	video_putc(c);
	return;

      case 3:                      /* FGA                                    */
	video_putc(c);
	return;

      default:                     /* Anything else is ignored               */
        return;
      }

    /* Deal with special characters                                          */
    switch(c) {
    case '\n':
        fcnt += LINE_LENGTH * 2;
        break;

    case '\b':
        fcnt = fcnt - 2;
        break;

    case '\r':
        if (fcnt > (2 * LINE_LENGTH)) {
            fcnt = (fcnt / (2 * LINE_LENGTH)) * 2 * LINE_LENGTH;
        }
        else {
            fcnt = 0;
        }
        break;

    default:
        print_char = 1;
        break;

    }
    if (fcnt >= TOTAL_SCREEN_BYTES) {
        scroll_up(base);
        fcnt = BEG_LAST_LINE;
    }

    if (print_char) {
        outbmem(base+fcnt, c);
        fcnt += 2;
    }

    /* Set the cursor position                                               */
    set_cursor(port, fcnt / 2);

    /* Save the new position of the next character                           */
    if (port == 1)
      fcntmono = fcnt;

    if (port == 2)
      fcntvga = fcnt;
}

scroll_up(unsigned int base)
{
    int         byte;
    int         i, j;

    for (i = BEG_2ND_LINE, j = 0;
         i < TOTAL_SCREEN_BYTES;
         i=i+2, j=j+2) {

      outbmem(base + j, inbmem(base + i));
    }

    for (i = BEG_LAST_LINE;
         i < TOTAL_SCREEN_BYTES;
         i=i+2) {
        outbmem(base + i, ' ');
    }
}

