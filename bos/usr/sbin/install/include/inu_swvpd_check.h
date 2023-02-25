/* @(#)12  1.3  src/bos/usr/sbin/install/include/inu_swvpd_check.h, cmdinstl, bos411, 9428A410j 6/17/93 16:07:31 */
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: inu_swvpd_check.h
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

#ifndef __H_INUSWVPDCK
#define __H_INUSWVPDCK

/*
** Values returned by inu_swvpd_check
*/

#define SUCCESS		0	/* Successful completion!						*/
								/* The SWVPD  contains both the LPP and   */
								/* PRODUCT records for this lpp-option.   */

#define FAILURE		-1	/* The SWVPD for the specified option is  */
								/* missing one or both of the above       */
								/* records.                               */
 


/*
** Syntax:  rc = inu_swvpd_check(op)
**
** Input parameters:
**
**	op:	pointer to an lpp-option structure. The data type of this 
** 	      structure is Option_t, which is declared in inu_toc.h.
**            
** Output parameters:
**
**	None.
**
** Return values:
**
**  Returns SUCCESS if both the option's LPP and PRODUCT SWVPD
**  records exist and FAILURE otherwise.
** 
**
** lpp-option structure fields used/modified:
**
**      name:  contains the name of the lpp-option (input only).
**
**      level.ver: contains the version of the lpp-option (input only)
**
**      level.rel: contains the release of the lpp-option (input only)
**
**      level.mod: contains the modification level of the lpp-option (input only)
**
**      level.fix: contains the fix level of the lpp-option (input only)
*/

int inu_swvpd_check(Option_t *first);


#endif
