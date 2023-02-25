/* @(#)22	1.5  src/tenplus/sf_progs/mri.h, tenplus, tenplus411, GOLD410 7/18/91 13:53:08 */

/*
 * COMPONENT_NAME: (INED)  INed Editor
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
 /***************************************************************************/
 /* mri.h -- constants used in converting structured files to and from */
 /* MRI message catalog (.msg format) files */
 /* The information here does not apply to MRI catalogs in general; */
 /* rather it applies to the special MRI catalogs that we build to */
 /* hold information about INed structured files. */
 /***************************************************************************/

#ifndef	_MRI_H
#define _MRI_H

#include <chardefs.h>

/* Prefix strings for MRI file lines */
#define HCOMMENT	"$ $"	/* This indicates that a line is a */
				/* hidden comment in the MRI catalog */
#define	PREFIX_LEN	3	/* length of above strings */

#define MRI_COMMAND_CHAR	'$'	/* Character used in MRI */
					/* catalogs to introduce an */
					/* MRI command */
#define	MRI_COMMENT_CHAR	SPACE	/* When following the command */
					/* char, indicates line is a */
					/* visible comment */

/* Flags to follow the prefix string */
#define COMMENT_FLAG	'*'	/* line should be ignored by mri2sf */
#define NAME_FLAG	'N'	/* line is the name of the structured */
				/* file */ 
#define QUOTE_FLAG	'"'	/* line is a quoted string (will have */
				/* trailing quote, as well) */ 
#define TYPE_FLAG	'%'	/* line is type of node */
#define FLAGS_FLAG	'.'	/* line is flag bits for node */
#define PATH_FLAG	':'	/* line is the data path for info */
				/* presented just above here */ 
#define ARRAY_END_FLAG	'$'	/* indicates the end of an array */

#endif	/* _MRI_H */
