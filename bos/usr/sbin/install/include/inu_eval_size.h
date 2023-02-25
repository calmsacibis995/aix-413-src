/* @(#)77       1.2  src/bos/usr/sbin/install/include/inu_eval_size.h, cmdinstl, bos411, 9428A410j 6/10/93 18:34:14 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define MAXFILESYSTEMS 256
#include <sys/types.h>
#include <macros.h>

extern char **fs;
extern int *fs_flags;
extern size_t *fs_total;
extern size_t *al_total;
extern size_t *work_max;
extern int nfs;




