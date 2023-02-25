# @(#)85        1.7  src/packages/bos/rte/misc_cmds/Makefile, pkgbossub, pkg411, GOLD410 6/22/94 11:52:21
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

OPTIONS		= bos.rte.misc_cmds

FILESET_BASE	= misc_cmds
SCRIPTS		= bos.rte.misc_cmds.add
OTHERS		= swvpd.make sysck.make

USR_ODMADD_LIST += cmdcomm.add%bos.rte.misc_cmds \
		   cmduname.add%bos.rte.misc_cmds

.include <../subsys.mk>
.include <${RULES_MK}>
