# @(#)15        1.9  src/packages/bos/rte/control/Makefile, pkgbossub, pkg41J, 9523A_all 6/5/95 16:08:36
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

OPTIONS		= bos.rte.control

ROOT_OPTIONS	= bos.rte.control

FILESET_BASE	= control
SCRIPTS		= bos.rte.control.add
OTHERS		= swvpd.make swvpdroot.make sysck.make

USR_ODMADD_LIST += cmdcntl.add%bos.rte.control \
		   cmdoper.add%bos.rte.control

.include <../subsys.mk>
.include <packdep.mk>
.include <${RULES_MK}>
