/* @(#)71  1.3 src/bos/usr/sbin/install/include/inuconvert.h, cmdinstl, bos411, 9428A410j 6/17/93 16:07:41 */
#ifndef __H_INUCONVERT
#define __H_INUCONVERT
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: inuconvert.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
** Values returned by inuconvert
*/

#define SUCCESS		0	/* Successful completion! Conversion      */
				/* of individual lpp-options may have     */
				/* failed. The status of each             */
				/* lpp-option is recorded in its          */
				/* associated Option_t structure          */
				/* (which is a parameter to inuconvert).  */

#define FAILURE		-1	/* inuconvert encountered a terminating   */
				/* error. No lpp-options were             */
				/* successfully converted.                */
 


/*
** Syntax:  inuconvert(first, last, expand)
**
** Input parameters:
**
**      first: pointer to a structure which specifies the first lpp-option that
**             must be converted. The caller must put all lpp-option structures
**             that must be converted in a linked list. The data type of the
**             lpp-option structure is Option_t, which is declared in inu_toc.h.
**
**      last:  pointer to a structure which specifies the last lpp-option
**             in the above menntioned linked list that must be converted.
**
**      expand: specifies whether filesystem expansion is permitted.
**
** Return values:
**
**      Returns FAILURE if something is really hosed and no lpp-options
**      be converted. Returns SUCCESS if no terminating error was
**      encountered (some or all of the lpp-options may have failed, so
**      you must traverse the lpp-option list to determine which ones were
**      successfully converted).
**
** lpp-option structure fields used/modified:
**
**      name:  contains the name of the lpp-option (input only).
**
**      content: if the lpp-option has a root part that must be installed,
**               inuconvert will set this field to CONTENT_BOTH. Otherwise,
**               this field will not be modified (output only).
**
**      Status:  inuconvert sets this to STAT_SUCCESS if it successfully
**               converts the lpp-option. Otherwise, it is set to
**               STAT_FAILURE_INUCONVERT.
**
**      level.ver: contains the version of the lpp-option (input only)
**
**      level.rel: contains the release of the lpp-option (input only)
**
**      level.mod: contains the modification level of the lpp-option (input only)
**
**      level.fix: contains the fix level of the lpp-option (input only)
*/

int inuconvert(Option_t *first,
               Option_t *last,
               unsigned char expand);


#endif
