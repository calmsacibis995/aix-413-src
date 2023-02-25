#if !defined(lint)
static char sccsid[] = "@(#)75	1.6  src/tenplus/lib/util/ghost.c, tenplus, tenplus411, GOLD410 3/23/93 12:09:00";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	ghost
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File: ghost.c.                                                           */
/****************************************************************************/

#include <stdio.h>
#include <database.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/****************************************************************************/
/* ghost: tries to fix a broken .index file by ghost'ing it.                */
/****************************************************************************/

int ghost(char * file )
{
    register int i;
    auto     int status;
    auto     int pid;

    pid = fork();
    if (pid == 0) {  /* child - run ghost        */

	for (i = 0; i < _NFILE; i++)
	    (void)close( i );

	(void)execlp( "ghost", "ghost", file, (char *)NULL );
	exit( -1 );
    }

    if (wait( &status ) != pid)
	return( ERROR );

    return( ((status >> 8) != 0) ? ERROR : RET_OK );
}
