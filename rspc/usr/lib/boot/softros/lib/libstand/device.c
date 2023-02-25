static char sccsid[] = "@(#)15	1.2  src/rspc/usr/lib/boot/softros/lib/libstand/device.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:31";
#ifndef lint
#endif
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: close
 *		floppy_read
 *		floppy_write
 *		lseek
 *		open
 *		read
 *		write
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
#ifdef DEBUG

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <types.h>
#include <errno.h>
#include <stand.h>

#ifdef DEBUG	/* floppy support only when DEBUG */
unsigned short 	flop_rdwr( unsigned char, unsigned short, unsigned short,
			   unsigned char *, unsigned char );
unsigned char	flop_reset( unsigned char );
unsigned char	flop_turn_off_motor( unsigned char );
#endif /*DEBUG*/

void	ConsoleWrite( int, char );
char	ConsoleRead( int );
int	ConsoleInit( int );


/*---------------------------------------------------------------------------
 *  Device Descriptor Table:
 */

DevDesc	_iob[ _NIOBRW ] = {
    { TRUE,  (caddr_t)0, O_RDONLY, CONSOLE, 0, 0 },	/* stdin */
    { TRUE,  (caddr_t)0, O_WRONLY, CONSOLE, 0, 0 },	/* stdout */
    { TRUE,  (caddr_t)0, O_WRONLY, CONSOLE, 0, 0 },	/* stderr */
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 },
    { FALSE, (caddr_t)0,        0,       0, 0, 0 } 
};
	

/*---------------------------------------------------------------------------
 *  Floppy Read:
 *
 *	Purpose:   The goal is to fill passed buf with specified no. of bytes 
 *		   (nbyte). 
 *
 *	Algorithm: If address from device's descriptor is NOT on
 *		   sector boundary then we have to malloc sectorsize memory,
 *		   read the sector into that memory, and xfer the offset no.
 *		   of bytes to buffer.
 *
 *		   Then, if there are one or more multiples of sectorsize
 *		   in the remaining nbytes value we read that many sector(s)
 *		   directly into buffer.
 *
 *		   Finally, if there's more bytes to read (less than
 *		   sectorsize) we have to malloc sectorsize memory, read the
 *		   sector into that memory, and xfer the remaining no. of 
 *		   bytes to read to buffer.
 *
 *      Note:	   Does NOT handle case where attempt is made to read past
 *		   end of media.
 *
 */

int floppy_read( int fildes,		/* floppy file descriptor	 */
		 void *buf, 		/* points to memory area to fill */
		 unsigned int nbyte 	/* used - # of bytes to read 	 */
               )
    {
	int		sectorsize, drive;
        unsigned int 	read_count, copy_count, return_count;
        int 		i, current_sector, current_offset;
 	unsigned char 	*p_temp, *p_buf;
        int		sectors_to_read;
	int		rc;

	/*------------------------------------------------------*/
	/* Initialize parameters;				*/
	/* 	read_count:	tracks # of bytes to be read	*/
	/* 	current_sector:	tracks sector being read	*/
	/*	current_offset:	offset into current_sector	*/
	/*	return_count:	actual # of read bytes		*/
	/*------------------------------------------------------*/
	p_buf		= buf;
        sectorsize 	= SectorSize( DDESC.dev );
        drive   	= minor( DDESC.dev );
        read_count	= nbyte;
        current_sector	= ((int)(DDESC.addr)) / sectorsize;
	current_offset  = (int)DDESC.addr - (current_sector * sectorsize);
	return_count    = 0;

#if FLOPPY_DEBUG
printf("\n");
printf("p_buf = %lx\n", p_buf);
printf("sectorsize = %ld\n", sectorsize);
printf("drive = %ld\n", drive);
printf("read_count = %ld\n", read_count);
printf("current_sector = %ld\n", current_sector);
printf("current_offset = %ld\n", current_offset);
#endif

      /*----------------------------------------------------------------*/
      /* If addr not on sector boundary, read sector and transfer	*/
      /* portion to passed buffer					*/
      /*----------------------------------------------------------------*/
      if (current_offset)
	{
#if FLOPPY_DEBUG
printf("Running part #1 of floppy_read() code\n");
#endif
	   p_temp = (unsigned char *)malloc( sectorsize );  
			/* Get memory for sector */  
	   if (! p_temp )
		{
		errno = ENOMEM;	 	/* out of heap space */
		return(-1);
		}

           rc = flop_rdwr( drive, current_sector, 1, p_temp, O_RDONLY );
						   /* Read the sector */
           if ( rc != 1 )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   if  ((current_offset + read_count) <= sectorsize)
		copy_count = read_count;
	   else
		copy_count = sectorsize - current_offset;
	   
	   for (i=0; i<copy_count; i++)		       /* Xfer data from temp */
		*p_buf++ = *(p_temp+current_offset+i); /* to buffer	    */

	   free( p_temp );

	   read_count -= copy_count;
	   current_sector++;
	   return_count += copy_count;
	}


      /*----------------------------------------------------------------*/
      /* Read sectors that ARE on sector boundaries			*/
      /*----------------------------------------------------------------*/
      if (read_count/sectorsize)
        {
#if FLOPPY_DEBUG
printf("Running part #2 of floppy_read() code\n");
#endif
	   sectors_to_read = read_count/sectorsize;
           rc = flop_rdwr( drive, current_sector, 
				sectors_to_read, p_buf, O_RDONLY );
						   /* Read sector(s) */
           if ( rc != sectors_to_read )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   read_count -= (sectorsize * sectors_to_read);
	   current_sector += sectors_to_read;
	   p_buf += (sectorsize * sectors_to_read);
	   return_count += (sectorsize * sectors_to_read);
        }


      /*----------------------------------------------------------------*/
      /* Read final sector portion                               	*/
      /*----------------------------------------------------------------*/

      if (read_count)     /* additional portion of a sector to read ??? */
        {
#if FLOPPY_DEBUG
printf("Running part #3 of floppy_read() code\n");
#endif
	   p_temp= (unsigned char *)malloc( sectorsize );	 
			/* Create temp space */
	   if (! p_temp )
		{
		errno = ENOMEM;	   /* out of heap space */
		return(-1);
		}

           rc = flop_rdwr( drive, current_sector, 1, p_temp, O_RDONLY );
						   /* Read the sector */
           if ( rc != 1 )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   for (i=0; i<read_count; i++)	       	/* Xfer data from temp */
		*p_buf++ = *(p_temp+i); 	/* to buffer	    */

	   free( p_temp );

	   return_count += read_count;

	}

	
      /*----------------------------------------------------------------*/
      /* Update file descriptor                                  	*/
      /*----------------------------------------------------------------*/
#if FLOPPY_DEBUG
printf("Updating floppy file descriptor..\n");
#endif
      DDESC.addr += nbyte;

#if FLOPPY_DEBUG
printf("Return count is: %d\n", return_count);
#endif
      return (return_count);

    }  /* End of 'floppy_read' */

/*---------------------------------------------------------------------------
 *  Floppy Write:
 *
 *	Purpose:   The goal is to write specified no. of bytes to disk from   
 *		   buf.
 *
 *	Algorithm: If address from device's descriptor is NOT on
 *		   sector boundary then we have to malloc sectorsize memory,
 *		   read the sector into that memory, copy the offset no.
 *		   of bytes to malloc'ed area, and write back the sector to
 *		   disk.
 *
 *		   Then, if there are one or more multiples of sectorsize
 *		   in the remaining nbytes value we write from buffer
 *		   directly to those sectors.
 *
 *		   Finally, if there's more bytes to write (less than
 *		   sectorsize) we have to malloc sectorsize memory, read the
 *		   sector into that memory, xfer the remaining no. of 
 *		   bytes to the malloc'ed area, and write the sector back to
 *		   disk.
 *
 *      Note:	   Does NOT handle case where attempt is made to write past
 *		   end of media.
 *
 */

int floppy_write(int fildes,		/* floppy file descriptor	 */
		 void *buf, 		/* points to mem area to put on disk */
		 unsigned int nbyte 	/* used - # of bytes to write	 */
               )
    {
	int		sectorsize, drive;
        unsigned int 	write_count, copy_count, return_count;
        int 		i, current_sector, current_offset;
        unsigned char 	*p_temp, *p_buf;
        int		sectors_to_write;
	int		rc,wc;

	/*------------------------------------------------------*/
	/* Initialize parameters;				*/
	/* 	write_count:	tracks # of bytes to be written */
	/* 	current_sector:	tracks sector being written	*/
	/*	current_offset:	offset into current_sector	*/
	/*	return_count:	actual # of written bytes	*/
	/*------------------------------------------------------*/
	p_buf		= buf;
        sectorsize 	= SectorSize( DDESC.dev );
        drive   	= minor( DDESC.dev );
        write_count	= nbyte;
        current_sector	= ((int)(DDESC.addr)) / sectorsize;
	current_offset  = (int)DDESC.addr - (current_sector * sectorsize);
	return_count    = 0;

#if FLOPPY_DEBUG
printf("\n");
printf("p_buf = %lx\n", p_buf);
printf("sectorsize = %ld\n", sectorsize);
printf("drive = %ld\n", drive);
printf("write_count = %ld\n", write_count);
printf("current_sector = %ld\n", current_sector);
printf("current_offset = %ld\n", current_offset);
#endif

      /*----------------------------------------------------------------*/
      /* If addr not on sector boundary, read sector, copy portion, and */
      /* write sector back to disk.					*/
      /*----------------------------------------------------------------*/
      if (current_offset)
	{
#if FLOPPY_DEBUG
printf("Running part #1 of floppy_write() code\n");
#endif
	   p_temp = (unsigned char *)malloc( sectorsize );  
			/* Get memory for sector */  
	   if (! p_temp )
		{
		errno = ENOMEM;	 	/* out of heap space */
		return(-1);
		}

           rc = flop_rdwr( drive, current_sector, 1, p_temp, O_RDONLY );
						   /* Read the sector */
           if ( rc != 1 )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   if  ((current_offset + write_count) <= sectorsize)
		copy_count = write_count;
	   else
		copy_count = sectorsize - current_offset;
	   
	   for (i=0; i<copy_count; i++)		       /* Xfer data from buf */
		*(p_temp+current_offset+i) = *p_buf++; /* to temp  */

           wc = flop_rdwr( drive, current_sector, 1, p_temp, O_WRONLY );
						   /* Write the sector */
	   free( p_temp );

           if ( wc != 1 )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   write_count -= copy_count;
	   current_sector++;
	   return_count += copy_count;
	}


      /*----------------------------------------------------------------*/
      /* Write sectors which ARE on sector boundaries.			*/
      /*----------------------------------------------------------------*/
      if (write_count/sectorsize)
        {
#if FLOPPY_DEBUG
printf("Running part #2 of floppy_write() code\n");
#endif
	   sectors_to_write = write_count/sectorsize;
           wc = flop_rdwr( drive, current_sector, 
				sectors_to_write, p_buf, O_WRONLY );
						   /* Write sector(s) */
           if ( wc != sectors_to_write )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   write_count -= (sectorsize * sectors_to_write);
	   current_sector += sectors_to_write;
	   p_buf += (sectorsize * sectors_to_write);
	   return_count += (sectorsize * sectors_to_write);
        }


      /*----------------------------------------------------------------*/
      /* Read final sector portion                               	*/
      /*----------------------------------------------------------------*/

      if (write_count)     /* additional portion of a sector to write ??? */
        {
#if FLOPPY_DEBUG
printf("Running part #3 of floppy_write() code\n");
#endif
	   p_temp= (unsigned char *)malloc( sectorsize );	 
			/* Create temp space */
	   if (! p_temp )
		{
		errno = ENOMEM;	   /* out of heap space */
		return(-1);
		}

           rc = flop_rdwr( drive, current_sector, 1, p_temp, O_RDONLY );
						   /* Read the sector */
           if ( rc != 1 )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   for (i=0; i<write_count; i++)	/* Xfer data from buf  */
		*(p_temp+i) = *p_buf++;		/* to temp */

           wc = flop_rdwr( drive, current_sector, 1, p_temp, O_WRONLY); 
						   /* Write the sector */
	   free( p_temp );

           if ( wc != 1 )
                {
                  errno = EIO;     /* device I/O error */
                  return(-1);
                }

	   return_count += write_count;

	}

	
      /*----------------------------------------------------------------*/
      /* Update file descriptor                                  	*/
      /*----------------------------------------------------------------*/
#if FLOPPY_DEBUG
printf("Updating floppy file descriptor..\n");
#endif
      DDESC.addr += nbyte;

#if FLOPPY_DEBUG
printf("Return count is: %d\n", return_count);
#endif
      return (return_count);

    }  /* End of 'floppy_write()' */


/*---------------------------------------------------------------------------
 *  Device Open:
 */

int open( dev_t dev, int oflag, mode_t perms )
    {
	int	fildes;

	for ( fildes = 0; fildes < _NIOBRW; fildes++ )	/* find empty slot */
	    if ( DDESC.open == FALSE )
		{
		DDESC.open   = TRUE;		/* then take it */
		DDESC.addr   = NULL;
		DDESC.oflags = oflag;
		DDESC.dev    = dev;
		break;				/* exit loop */
	        }
	if ( fildes == _NIOBRW )		/* couldn't find a free slot */
	    {
	    errno = ENFILE;			/* too many open files */
	    return( -1 );
	    }

	switch ( major( dev ))			/* select device */
	    {
	    case HARDDISK:
	    case CD_ROM:
	    case NETWORK:
	    default:
		errno = ENXIO;			/* not yet supported */
		return( -1 );
		break;

#ifdef DEBUG
	    case FLOPPY:
		if (! flop_reset( minor( dev )))
		    {
		    DDESC.open = FALSE;
		    errno = EIO;		/* floppy I/O error */
		    return( -1 );
		    }
		break;
#endif

	    case CONSOLE:
		if (! ConsoleInit( fildes ))
		    {
		    DDESC.open = FALSE;
		    errno = EIO;		/* console I/O error */
		    return( -1 );
		    }
		break;
	    }
	return( fildes );			/* file descriptor is index */
    }

/*---------------------------------------------------------------------------
 *  Device Read:
 */

int read( int fildes, void *buf, unsigned int nbyte )
    {
	int	i, count = 0;
	char	c, *ptr = buf;

	if ( fildes > _NIOBRW || fildes < 0 )
	    {
	    errno = EBADF;
	    return( -1 );
	    }
	if ( DDESC.oflags & O_WRONLY )	/* write-only? */
	    {
	    errno = EACCES;
	    return( -1 );
	    }
	switch ( major( DDESC.dev ))	/* select device type */
	    {
#ifdef DEBUG
	    case FLOPPY:
		count = floppy_read( fildes, buf, nbyte );
		break;
#endif

	    case CONSOLE:
		for ( i = 0; i < nbyte; i++ )
		    {
		    c = ConsoleRead( fildes );
		    if ( c < 0 )		/* if error */
			return( -1 );		/* return error */
		    else
		        count++, *ptr++ = c;	/* else store char */
		    }
		break;

	    default:			/* how could this happen? */
		return( 0 );
	    }
	return( count );
    }


/*---------------------------------------------------------------------------
 *  Device Write:
 */

int write( int fildes, const void *buf, unsigned int nbyte )
    {
	int	i, count = 0;
	char	*ptr = (char *)buf;

	if ( fildes > _NIOBRW || fildes < 0 )
	    {
	    errno = EBADF;
	    return( -1 );
	    }
	if (! DDESC.oflags & ( O_WRONLY | O_RDWR ))	/* read-only? */
	    {
	    errno = EACCES;
	    return( -1 );
	    }
	switch ( major( DDESC.dev ))	/* select device type */
	    {
#ifdef DEBUG
	    case FLOPPY:
		count = floppy_write( fildes, (void *)buf, nbyte );
		break;
#endif /*DEBUG*/

	    case CONSOLE:
		for ( i = 0; i < nbyte; i++ )
		    {
		    ConsoleWrite( fildes, *ptr++);
		    }
		break;

	    default:			/* how could this happen? */
		return( 0 );
	    }
	return( count );
    }


/*---------------------------------------------------------------------------
 *  Device Close:
 */

int close( int fildes )
    {
	if ( fildes > _NIOBRW || fildes < 0 )
	    {
	    errno = EBADF;
	    return( -1 );
	    }
	if ( DDESC.open == FALSE )
	    return( 0 );
	switch ( major( DDESC.dev ))	/* select device type */
	    {
#ifdef DEBUG
	    case FLOPPY:
		if (! flop_turn_off_motor (minor( DDESC.dev )))
		    {
		    errno = EIO;
		    return( -1 );
		    }
		break;
#endif /*DEBUG*/

	    default:
	        errno = ENODEV;
	        return( -1 );
	    }
	DDESC.open = FALSE;
	return( 0 );
    }

/*---------------------------------------------------------------------------
 *  Device Logical Seek:
 */

off_t lseek( int fildes, off_t offset, int whence )
    {
	if ( fildes > _NIOBRW || fildes < 0 )
	    {
	    errno = EBADF;
	    return( -1 );
	    }
	switch ( whence )
	    {
	    case SEEK_SET:
		DDESC.addr = (caddr_t)offset;
		break;

	    case SEEK_CUR:
		DDESC.addr += offset;
		break;

	    default:
		errno = EINVAL;
		return(-1);
	    }
	return( (off_t)DDESC.addr );
    }
#endif /*DEBUG*/
