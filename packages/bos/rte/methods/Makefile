# @(#)82        1.8  src/packages/bos/rte/methods/Makefile, pkgbossub, pkg411, GOLD410 6/23/94 13:24:40
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
VPATH		= ..:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

RTE_TCB		= true

OPTIONS		= bos.rte.methods

FILESET_BASE	= methods
SCRIPTS		= bos.rte.methods.add
OTHERS		= swvpd.make sysck.make

USR_ODMADD_LIST += sm_dev.add%bos.rte.methods \
		   sm_sys.add%bos.rte.methods

.include <../subsys.mk>
.include <${RULES_MK}>
