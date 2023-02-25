/* @(#)83       1.1  src/rspc/kernext/pm/wakeup/debug.h, pwrsysx, rspc41J, 9510A 3/8/95 06:44:09 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for debug function control. 
 *   If PM_DEBUG flag is not specified, all debug functions are 
 *   disabled without source code changes.
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_H_DEBUG
#define _H_DEBUG

#ifdef 	PM_DEBUG

#define DbgInit		HrInitConsole
#define DbgPrint(x)	HrPrint x

#else

#define DbgInit()
#define DbgPrint(x) 

#endif

#endif

