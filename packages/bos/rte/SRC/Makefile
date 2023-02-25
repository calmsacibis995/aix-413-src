# @(#)88        1.9  src/packages/bos/rte/SRC/Makefile, pkgbossub, pkg41J, 9507C 2/1/95 15:49:51
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
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ..:root:${MAKETOP}/bos/objclass

RTE_TCB		= true

OPTIONS		= bos.rte.SRC

ROOT_OPTIONS	= bos.rte.SRC

FILESET_BASE	= SRC
SCRIPTS		= bos.rte.SRC.add
OTHERS		= swvpd.make swvpdroot.make sysck.make

USR_ODMADD_LIST += cmdsrc.add%bos.rte.SRC

.include <packdep.mk>
.include <../subsys.mk>
.include <${RULES_MK}>
