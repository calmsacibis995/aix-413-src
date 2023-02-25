# @(#)36        1.8  src/packages/bos/rte/filesystem/Makefile, pkgbossub, pkg411, GOLD410 6/22/94 11:49:31
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
VPATH		= ..:root:${MAKETOP}/bos/objclass

RTE_TCB		= true

OPTIONS		= bos.rte.filesystem

ROOT_OPTIONS	= bos.rte.filesystem

FILESET_BASE	= filesystem
SCRIPTS		= bos.rte.filesystem.add
OTHERS		= swvpd.make swvpdroot.make sysck.make

USR_ODMADD_LIST += cmdfs.add%bos.rte.filesystem

.include <../subsys.mk>
.include <${RULES_MK}>
