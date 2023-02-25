#if !defined(lint)
static char sccsid[] = "@(#)87	1.5  src/tenplus/helpers/dir/externs.c, tenplus, tenplus411, GOLD410 7/18/91 12:39:32";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 *                      Copyrighted as an unpublished work.                 
 *                         INTERACTIVE TEN/PLUS System                      
 *                             TEN/PLUS File Manager                        
 *              (c) Copyright INTERACTIVE Systems Corp. 1983, 1988          
 *                             All rights reserved.                         
 *                                                                          
 *   RESTRICTED RIGHTS                                                      
 *   These programs are supplied under a license.  They may be used and/or  
 *   copied only as permitted under such license agreement.  Any copy must  
 *   contain the above notice and this restricted rights notice.  Use,      
 *   copying, and/or disclosure of the programs is strictly prohibited      
 *   unless otherwise provided in the license agreement.                    
 *                                                                          
 *   TEN/PLUS is a registered trademark of INTERACTIVE Systems Corporation  
 *	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
 */

#if !defined(lint)
static char copyright[] =
	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
#endif /* lint */

/****************************************************************************/
/* File: externs.c - definitions for global variables                       */
/****************************************************************************/

#include <limits.h>
#include <database.h>
#include "index.h"

ASTRING      Current_directory;          /* Directory being edited          */
int          Display_state;              /* Either hidden or not            */
int          Form_type = UNKNOWN_FORM;   /* Which form is being viewed      */
ASTRING      Index_file_name;            /* Name of the index file          */
char         Last_stat_file[PATH_MAX];    /* Name of last file stat'ed       */
ASTRING      Put_directory;              /* Save dir. for deleted files     */
int          Pd_exists;                  /* TRUE if put directory is usable */
int          Result_of_stat;             /* Result of the last stat         */
struct stat  Stat_buffer;                /* Result of last call to stat     */
int          Writable_directory;         /* Current_directory writable?     */

