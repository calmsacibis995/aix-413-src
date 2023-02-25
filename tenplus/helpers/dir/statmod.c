#if !defined(lint)
static char sccsid[] = "@(#)07	1.7  src/tenplus/helpers/dir/statmod.c, tenplus, tenplus411, GOLD410 3/23/93 11:56:28";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	check_bits
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
/* File: statmod.c - hides and buffers calls to stat.                       */
/****************************************************************************/

#include <database.h>
#include "index.h"
#include <sys/types.h>
#include "statmod.h"
#include <unistd.h>

/****************************************************************************/
/* check_bits: returns TRUE if the file permissions allow the operation.    */
/****************************************************************************/

int check_bits( char *file_name, int operation )
{
    register int uid;
    register int mode;

#ifdef MAXDEBUG
    debug( "check_bits( file_name = '%s', operation = %d )", file_name,
	   operation );
#endif

    if (is_not_file( file_name ))
	return( FALSE );

    uid = geteuid();
    if ( uid == ROOT )
	return( TRUE );

    mode = Stat_buffer.st_mode;
    if (mode == (ushort) ERROR)
	return( FALSE );

    /* Check if the user is the owner and if the owner can do the operation */

    if ( uid == Stat_buffer.st_uid )
	return( mode & operation );

    /* Check if the user is in the owner's group and */
    /*   if members of the owner's group can do the operation   */

    if ( getegid() == Stat_buffer.st_gid )
	return( mode & (operation >> 3) );

    /* Check if anybody can do the operation */

    return( mode & (operation >> 6) );
}
