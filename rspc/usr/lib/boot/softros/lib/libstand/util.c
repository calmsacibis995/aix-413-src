static char sccsid[] = "@(#)19	1.3  src/rspc/usr/lib/boot/softros/lib/libstand/util.c, rspc_softros, rspc411, GOLD410 6/13/94 16:10:22";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: _moveeq
 *		bcopy
 *		bzero
 *		copy_args
 *		delay
 *		display_memory
 *		sbrk
 *		set_progp
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

#include <stand.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *bcopy( char *src, char *dest, int len )
    {
        return( memcpy( dest, src, len ));
    }

void bzero( char *ptr, int len )
    {
        memset( ptr, 0, len );
    }

void delay( long ms )                   /* delay in milliseconds */
    {
        int     i;

        while( ms-- )                   /* for each millisecond */
            for( i = 0; i < 400; i++ )  /* count up to 400 */
                ;                       /* in a spin loop */
    }

                                        /* needed by the compiler */
void _moveeq( void *s1, const void *s2, size_t n )
    {
        memmove( s1, s2, n );
    }

/*--------------------------------------------------------------------------
 *  SBRK:
 *
 *  This version of sbrk assumes the following standalone memory map:
 *
 *                High RAM      +-----------------------+
 *                              |        Stack          |
 *              top of stack    |                       |
 *                  |           |- - - - - - - - - - - -|
 *                  V           |     Guard Band        |
 *                              |-----------------------|
 *                              |                       |
 *                              |     Memory Heap       |
 *                              |                       |
 *                              +-----------------------+
 *                              |                       |
 *                              |   Execution Image     |
 *                              |                       |
 *                              +-----------------------+
 *                              |         IVT           |
 *                 Low RAM      +-----------------------+
 *
 *   Memory is allocated starting from the bottom of the heap (at the end
 *   of the execution image); sbrk climbs up through the memory heap as
 *   each call to sbrk is made.  When sbrk reaches the guard band before
 *   the top of the stack, it aborts and returns NULL, protecting the stack.
 *   The global variable _progp (set up in crt0.o) points to a structure that
 *   contains the size and start address of the currently executing image.
 *   Note that execution images can be nested; i.e., the first image can
 *   malloc an area of memory, load in an image and jump to it.  The
 *   called image then begins a new sbrk at its end in memory.
 */

#define GUARDBAND       4096            /* one page between mem and stack */
#define ADDR_640K       (640*1024)

#define MEMORY_HOLE                     /* for PC kludge-card machines */

static char     *_MemBreak = NULL;
prog_t          progp;                  /* prog_t structure pointer */

void set_progp( prog_t  *p )
    {
        memcpy( &progp, p, sizeof( prog_t ));
    }

char *sbrk( int n )
    {
        int     stack;                  /* used to get stack location */

        if (! _MemBreak )               /* if first time, */
            {                           /* initialize MemBreak */
            if (! progp.start )         /* if no progp, abort */
                return( NULL );
            _MemBreak = (char *)(progp.start + progp.size);
            }
        else                            /* too close to stack area? */
            {
            if ( (int)_MemBreak >= ((int)&stack - GUARDBAND ))
                {
                printf("\n SBRK ERROR: %08x run into stack at %08x!",
                        _MemBreak, &stack );
                return( NULL );         /* then abort */
                }
            _MemBreak += n;             /* otherwise, granted */
            }                           /* align on quadbyte boundary: */
        _MemBreak = (char *)((((long)_MemBreak >> 2) + 1) * 4);

#ifdef MEMORY_HOLE                      /* if near memory hole, jump to 1M */
        if (( (int)_MemBreak <= ADDR_640K ) &&
            ( (int)_MemBreak > ( ADDR_640K - GUARDBAND )))
            _MemBreak  = (char *)0x100000;
#endif
        return( _MemBreak );
    }


void display_memory ( char *buffer, int length, int offset )
    {
        int     i, j, limit, line = 0;

        limit = (length + 15) & ~0xF;           /* next highest mult of 16 */
        for (i = 0; i <= limit; i++)
            {
            if (!( i % 16 ))                    /* mult of 16? */
                {
                if ( i )                        /* not zeroth byte? */
                    {
                    printf("     ");            /* print ASCII of last 16 */
                    for (j = i - 16; j < i; j++)
                        {
                        if (j >= length)        /* only up to the length */
                            printf(" ");
                        else                    /* control chars = "." */
                            if (buffer[j] < 0x20 || buffer[j] > 0x7e)
                                printf(".");
                            else
                                printf("%c", buffer[j]);
                        }
                    }
                if (i != limit)                 /* no label at limit */
                    printf("\n   %08x: ", offset + i);
                }
            if (!( i % 4 ))                     /* add space every */
                printf(" ");                    /* four bytes */
            if (i >= length)
                printf("  ");                   /* print hex only up to */
            else                                /* length */
                printf("%.2x", buffer[i]);
            }
        printf("\n");
    }

/*----------------------------------------------------------------------*/
/*  copy_args                                                           */
/*                                                                      */
/*  PURPOSE:                                                            */
/*      Copy array of strings and string pointers to a target area.     */
/*      Return no. of strings in the source array.                      */
/*      End of list of strings is indicated by '\0'.                    */
/*                                                                      */
/*  PASSED:                                                             */
/*      char **argv - Ptr to array of strings                           */
/*                                                                      */
/*  RETURNED:                                                           */
/*      int *nargc - no. of non-null strings                            */
/*                                                                      */
/*----------------------------------------------------------------------*/

void copy_args( argv, nargc, nargv)
  char * argv[];        /* Used - source array of ptrs to strings */
  int  * nargc;         /* Returned - no. of args pointed to by argv */
  char * nargv[];       /* Returned - dest. array of ptrs to strings */
{
   char * s;
   int i,j;     /* 'i' indicates the string no. that is being copied */
                /* 'j' indicates the char in the string that is being copied */

   for (i=0, *nargc = 0; argv[i] != (char *)0; i++)     /* Count no. of     */
        (*nargc)++;                                     /* strings in argv  */

   s = (char *) &nargv[((*nargc)+1)];   /* Pt to where strings will go      */
   nargv[(*nargc)] = (char *)0;         /* Put null value in last dest. ptr */

   for (i=0;argv[i];i++) {              /* Copy the src ptrs/strings to dest */
      nargv[i] = s;                     /* Point to string destination start */

      for(j=0; argv[i][j]; j++) {       /* Loop to copy an entire string */
         *s = argv[i][j];               /* Copy a character */
         s++;                           /* Bump to next dest loc for char */
      }
      *s = '\0';                        /* Null terminate the string */
      s++;                              /* Point to next dest string space */
   }
}


