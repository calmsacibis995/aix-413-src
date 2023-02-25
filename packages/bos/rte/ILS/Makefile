# @(#)85        1.7  src/packages/bos/rte/ILS/Makefile, pkgbossub, pkg411, GOLD410 6/22/94 11:45:35
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
VPATH		= ..:${MAKETOP}/bos/objclass

OPTIONS		= bos.rte.ILS

FILESET_BASE	= ILS
SCRIPTS		= bos.rte.ILS.add
OTHERS		= swvpd.make

USR_ODMADD_LIST += cmdmsg.add%bos.rte.ILS \
		   sm_mle.add%bos.rte.ILS \
		   ILS.add%bos.rte.ILS

.include <../subsys.mk>
.include <${RULES_MK}>
