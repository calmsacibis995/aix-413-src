/* @(#)61       1.3  src/bos/usr/sbin/install/include/inu_vpd.h, cmdinstl, bos411, 9428A410j 6/17/93 16:07:35 */
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_vpd.h
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
#ifndef _H_INU_VPD
#define _H_INU_VPD

/*--------------------------------------------------------------------------*
 * DEFINES for the inu_vpd_set_states() parameter "action"
 *--------------------------------------------------------------------------*/
#define	SET_STATES_TO_HOLD	     0	
#define	SET_HOLD_STATES_TO_BROKEN    1
#define	DELETE_HOLD_STATES           2
#define	SET_HOLD_STATES_TO_PREVIOUS  3

#endif	/* _H_INU_VPD */
