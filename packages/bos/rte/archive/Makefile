# @(#)94        1.7  src/packages/bos/rte/archive/Makefile, pkgbossub, pkg411, GOLD410 6/22/94 11:47:12
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

RTE_TCB		= true

OPTIONS		= bos.rte.archive

FILESET_BASE	= archive
SCRIPTS		= bos.rte.archive.add
OTHERS		= swvpd.make sysck.make

USR_ODMADD_LIST += cmdarch.add%bos.rte.archive

.include <../subsys.mk>
.include <${RULES_MK}>
