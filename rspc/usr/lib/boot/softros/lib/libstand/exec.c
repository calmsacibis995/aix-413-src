#ifndef lint
static char sccsid[] = "@(#)16	1.1  src/rspc/usr/lib/boot/softros/lib/libstand/exec.c, rspc_softros, rspc411, GOLD410 4/18/94 15:49:39";
#endif
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: load_and_go
 *		prog_call
 *		prog_exec
 *		prog_load
 *		prog_run
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

#include <stddef.h>
#include <types.h>
#include <errno.h>
#include <stand.h>
#include <xcoff.h>

#define	  ENTRY_POINT_OFFSET  0			/* offset into function */
						/* descriptor for entry */
						/* point		*/
/*
 *---------------------------
 *   prog_exec()
 *
 *   Routine to load image, 
 *   jump to it, and never,
 *   return
 *---------------------------
 */
void prog_exec(	dev_t 		dev, 		/* used - device type	  */
		daddr_t 	daddr, 		/* used - dev start addr  */	
		size_t 		size, 		/* used - image size (bytes) */
		caddr_t		memaddr,	/* used - load address	  */
		char 		**argv, 	/* used */
		char 		**envp		/* used */
	      )
    {
	prog_t	progp;

	prog_load( dev, daddr, size, memaddr );

	progp.start	= (long)memaddr;	/* address of next image */
	progp.size     = size;			/* size of next image */

	(void) prog_run( &progp, argv, envp );	/* do it */
    }





/*
 *---------------------------
 *   prog_call()
 *
 *   Routine to load image, 
 *   jump to it, and return
 *---------------------------
 */
int  prog_call(	dev_t 		dev, 		/* used - device type	  */
		daddr_t 	daddr, 		/* used - dev start addr  */	
		size_t 		size, 		/* used - image size (bytes) */
		caddr_t		memaddr,	/* used - load address	  */
		char 		**argv, 	/* used */
		char 		**envp		/* used */
	      )
    {
	int	rc;
	prog_t	progp;

						/* otherwise, load it */
	prog_load( dev, daddr, size, memaddr );

	progp.start	= (long)memaddr;	/* address of next image */
	progp.size     = size;			/* size of next image */

	rc = prog_run( &progp, argv, envp );	/* run it */

	return( rc );
    }




/*
 *-------------------------------------
 *   prog_load()
 *
 *   Routine to load image, 
 *   from specified device address,
 *   to specified memory address
 *-------------------------------------
 */
int  prog_load(	dev_t 		dev, 		/* used - device type	  */
		daddr_t 	daddr, 		/* used - dev start addr  */	
		size_t 		size, 		/* used - image size (bytes) */
		caddr_t		memaddr		/* used - load address	  */
	      )
    {
	int	fd, rc;

	fd = open( dev, O_RDWR, 0 );		/* open device */
	if ( fd < 0 )				/* open fail */
	    return( -1 );

	rc = lseek( fd, daddr, SEEK_SET );	/* move to address */
	if ( rc < 0 )				/* seek fail */
	    return( -1 );

	rc = read( fd, memaddr, size );		/* read it in */

	close( fd );

	return( rc );
    }



/*
 *-------------------------------------
 *   load_and_go()
 *
 *   Prepares function descriptor using
 *   the passed address, and then jump
 *   to that address (required by
 *   AIX 3.2 binder).
 *-------------------------------------
 */

 struct fcndes {
    unsigned long pc,toc,env;
 } func_descriptor;

 extern void  (*promaddr)();

static
void load_and_go(   void 	(*start_addr)(), 	/* used */
		    prog_t 	*p, 			/* used */
		    char 	*argv[], 		/* used */
		    char 	*envp[]			/* used */
                )
{

    void (*jump_addr)() = (void (*)())&func_descriptor;

    func_descriptor.pc  = (unsigned long)start_addr;
    func_descriptor.toc = 0;
    func_descriptor.env = 0;

    jump_addr( argv, envp, promaddr, p );
}





/*
 *-------------------------------------
 *   prog_run()
 *
 *   Routine to execute loaded image
 *-------------------------------------
 */
int  prog_run(	prog_t		*p,		/* used - start/size of image*/
		char 		*argv[], 	/* used */
		char 		*envp[]		/* used */
	     )
    {

        /*------------------------*/
        /*  Variable declarations */
        /*------------------------*/
        AOUTHDR         *optional_header;
        char            *p_func_descriptor;
        long            entry_point;


        /*------------------*/
        /* Code starts here */
        /*------------------*/
        optional_header = (AOUTHDR *)(p->start + FILHSZ);

        /*------------------------------------------------------*/
        /* Now we are pointing to auxilliary header.            */
        /* Get address of entry point function descriptor.      */
        /*------------------------------------------------------*/
        p_func_descriptor   = (char *)(optional_header->entry);
        entry_point         = *(long *)
                                (ENTRY_POINT_OFFSET + (long)p_func_descriptor);

#if 0
printf("p->start is %lx\n", p->start);
printf("file_header is %lx\n", file_header);
printf("optional_header is %lx\n", optional_header);
printf("p_func_descriptor is %lx\n", p_func_descriptor);
printf("entry_point is %lx\n", entry_point);
#endif

        /*----------------------------------------------*/
        /* Now set up function descriptor in order to   */
        /* absolute jump to 'entry_point'               */
        /*----------------------------------------------*/

        load_and_go((void (*)())entry_point, p, argv, envp);

        return( 0 );
    }
