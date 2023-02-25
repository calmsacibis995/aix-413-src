static char sccsid[] = "@(#)09	1.2  src/rspc/usr/lib/boot/softros/lib/libc/stdlib.c, rspc_softros, rspc41J, 9523A_all 6/6/95 14:48:55";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: abs
 *		atoi
 *		if
 *		reverse
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>

#pragma options source
#pragma options list


static void reverse(char s[]){
   int c, i, j;
   for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
      c = s[i];
      s[i] = s[j];
      s[j] = c;
   }
}

#ifdef abs
#undef abs
#endif

int abs(int signed_value) {
   if (signed_value < 0) signed_value = -signed_value;
   return signed_value;
}

int atoi( const char *s ) {
   register int i = 0, n, sign = 1;

   for( i = 0; isspace(s[i]); i++ )        /* skip whitespace */
       ;
   if ( s[i] == '-' )                      /* negative sign? */
       sign = -1, i++;
   if ( s[i] == '+' )                      /* positive sign? */
       i++;
   for ( n = 0; isdigit( s[i] ); i++ )     /* build number */
       n = 10 * n + ( s[i] - '0' );

   return( sign * n );
}

// note, this has only been tested with radix of 10 (decimal) and 16 (decimal)
// for that reason, we're allowing only 10 or 16 as valid values.

char * itoa(int n, char s[], int radix) {
   int i=0, sign;
	if (radix != 16)
		/* allow only 10 or 16 as a radix value */
		radix = 10;
   if ((sign = n) < 0) n = -n;
   do {
      s[i] = n % radix;
      s[i] += (s[i++] <= 9) ? '0' : '7';
   } while ((n /= radix) > 0);
   if (sign < 0) s[i++] = '-';
   s[i] = '\0';
   reverse(s);
   return s;
}

long atol( const char *s )
    {
        register int i = 0, sign = 1;
        register long n;

        for( i = 0; isspace( s[i] ); i++ )      /* skip whitespace */
            ;
        if ( s[i] == '-' )                      /* negative sign? */
            sign = -1, i++;
        if ( s[i] == '+' )                      /* positive sign? */
            i++;
        for ( n = 0; isdigit( s[i] ); i++ )     /* build number */
            n = 10 * n + ( s[i] - '0' );

        return( sign * n );
    }


/*-----------------------------------------------------------------------
 *  Random number generator; this is coded directly from the C standard:
 */

static unsigned long _randnum = 1;      /* current random number */

int rand ( void )                       /* RAND_MAX assumed to be 32767 */
    {
        _randnum = _randnum * 1103515245 + 12345;
        return(( unsigned int )( _randnum >> 16 ) % RAND_MAX+1 );
    }

void srand ( unsigned int seed )
    {
        _randnum = seed;
    }

/*-----------------------------------------------------------------------
 *  STORAGE ALLOCATOR:
 *
 *  This uses a "first-fit" algorithm (rather than "best fit") to keep
 *  things simple.  Also, rather than set a minimum number of blocks to
 *  allocate, this malloc calls sbrk for only as much additional memory
 *  that is needed.
 *
 */

typedef struct mallocheader {
    struct mallocheader *next;          /* next chunk on free list */
    unsigned long       size;           /* size of current chunk in blocks */
    char                id[4];
    unsigned long       fill2;          /* Force 16 byte alignment              */

} MallocHeader;                         /* keep this on word boundary! */

#define BLOCKSZ         sizeof( MallocHeader )

static MallocHeader     MemBase;        /* anchor of malloc list */
       MallocHeader     *MemFree = NULL;/* pointer to free list */

void *malloc( size_t size )
    {
        MallocHeader *bp, *last, *newb;
        char *cp, *sbrk( int );
        unsigned int nblocks;

        if (! size )                    /* nothing yields nothing */
            return( NULL );

        nblocks = 1 + size / BLOCKSZ;   /* need header + mem area */
        if ( size % BLOCKSZ )           /* plus 1 more if fraction */
            nblocks += 1;

        if ( MemFree == NULL )          /* start free list? */
            {
            MemBase.next = MemFree = &MemBase;
            MemBase.size = 0;
            MemBase.id[0] = 'F';
            }
        last = MemFree;                 /* pickup from free list */
        bp = MemFree->next;

        while( TRUE )                           /* search loop */
            {
            if (bp->id[0] != 'F')
               bp->id[1] = 'X';                 /* Dummy code to set a breakpoint */

            if ( bp->size >= nblocks )          /* fit? */
                {
                if ( bp->size == nblocks )      /* exactly? */
                    last->next = bp->next;      /* then take it */
                else                            /* else, */
                    {                           /* split into two chunks */
                    bp->size -= nblocks;        /* reduce size of first chunk */
                    bp += bp->size;             /* point to new chunk */
                    bp->size = nblocks;         /* and fill it in */
                    }
                MemFree = last;                 /* update free list pointer */
                bp->id[0] = 'A';
                return ((void *)(++bp));        /* return chunk beyond hdr */
                }
            if ( bp == MemFree )                /* couldn't find anything? */
                {
                cp = sbrk( nblocks * BLOCKSZ ); /* allocate more memory */

                if (! cp )                      /* fail? */
                    return( NULL );             /* then we can't do it */

                newb = (MallocHeader *)cp;      /* create new block */
                newb->next = NULL;              /* detached from list */
                newb->size = nblocks;           /* size of block */
                newb->id[0] = 'A';
                newb->id[1] = '-';
                newb->id[2] = '-';
                free((void *)( newb + 1 ));     /* put it on free list */

                bp = MemFree;                   /* try again */
                }
            last = bp, bp = bp->next;           /* move to next block */
            }
    }

void *calloc( size_t nmemb, size_t size )
    {
        int numbytes = nmemb * size;            /* number of bytes */
        char *p;

        if ( p = malloc( numbytes ))            /* get memory */
            memset( p, 0, numbytes );           /* I'm not proud */
        return( p );                            /* return it */
    }

void *realloc( void *ptr, size_t size )
    {
        MallocHeader *p;
        char    *newp;
        unsigned int nblocks;

        nblocks = 1 + size / BLOCKSZ;   /* need header + mem area */
        if ( size % BLOCKSZ )           /* plus 1 more if fraction */
            nblocks += 1;

        if (! ptr )                             /* simple malloc? */
            return( malloc( size ));
        if (! size )                            /* simple free? */
            {
            free( ptr );
            return( NULL );
            }

        p = (MallocHeader *)ptr - 1;            /* get header */

        if ( p->size == nblocks)                /* essentially a no-op? */
            return( ptr );                      /* give it back */
        else                                    /* else, different size */
            {                                   /* this is inefficient */
            newp = malloc( size );              /* but easy to maintain: */
            if ( newp )
                memcpy( newp, ptr, (p->size-1)*(BLOCKSZ) );   /* copy it over */
            free( ptr );                        /* release old memory */
            return( newp );                     /* return new memory */
            }
    }

void free( void *area )
    {
        MallocHeader *chunkp, *listp;

        if ( !area )                            /* if bogus, ignore */
            return;

        listp = MemFree;                        /* start at free list */
        chunkp = (MallocHeader *)area - 1;      /* point to chunk header */

        if (chunkp->id[0] != 'A') {
           chunkp->id[2] = 'Y';
           return;                              /* Place to set a breakpoint */
        }                                       /* until between neighbors */
        chunkp->id[0] = 'F';

        while (!( chunkp > listp && chunkp < listp->next )) {

            if (chunkp == listp) {
               chunkp->id[2] = 'M';
               return;
            } /* endif */

            if ( listp >= listp->next ) /* if at end of list */
                                                /* and chunk is outside list, */
                if ( chunkp > listp || chunkp < listp->next )
                    break;                      /* needs to be added. */

            listp = listp->next;                /* search the list */
        }
                                                /* join to upper neighbor */
        if ( chunkp + chunkp->size == listp->next )  {
            chunkp->size += listp->next->size;
            chunkp->next  = listp->next->next;
        } else
            chunkp->next = listp->next;

        listp->next = chunkp;
                                                /* join to lower neighbor */
        if ( listp + listp->size == chunkp ) {
            listp->size += chunkp->size;
            listp->next  = chunkp->next;
        }
    }



/*-----------------------------------------------------------------------
 *  Exit code:
 */

#define MAXEXITS        32                      /* max number of atexits */

static void (*_atexit[ MAXEXITS ])(void);       /* atexit functions */
static size_t   _n_atexit = 0;                  /* number of functions */

void exit ( int status )
    {
        while ( _n_atexit-- )                   /* functions to call? */
            (*_atexit[ _n_atexit ])();          /* call them */

        __exit( status );                       /* return to calling prog */
    }

int atexit( void (*func)(void))
    {
        if ( _n_atexit == MAXEXITS-1 )          /* out of room? */
            return( -1 );                       /* return error */
        else
            _atexit[ _n_atexit++ ] = func;      /* else store function */
        return( 0 );                            /* no error */
    }
