# @(#)63        1.5  src/packages/bos/rte/ifor_ls/Makefile, pkgbossub, pkg41J, 9523A_all 6/5/95 16:08:53
#
# COMPONENT_NAME: pkgbossub
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ..:${MAKETOP}/bos/usr/bin/netls

OPTIONS		= bos.rte.ifor_ls

FILESET_BASE	= ifor_ls
SCRIPTS		= bos.rte.ifor_ls.add
OTHERS		= swvpd.make

USR_ODMADD_LIST += netls.add%bos.rte.ifor_ls

.include <../subsys.mk>
.include <packdep.mk>
.include <${RULES_MK}>
