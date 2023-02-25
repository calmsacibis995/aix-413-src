# @(#)94        1.10  src/packages/bos/rte/printers/Makefile, pkgbossub, pkg41J, 9523A_all 6/5/95 16:09:25
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
VPATH		 = ..:root:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

RTE_TCB		= true

OPTIONS		= bos.rte.printers

ROOT_OPTIONS	= bos.rte.printers

FILESET_BASE	= printers
SCRIPTS		= bos.rte.printers.add
OTHERS		= swvpd.make swvpdroot.make sysck.make sysckroot.make

USR_ODMADD_LIST += cmdque.add%bos.rte.printers \
		   qdaemon.add%bos.rte.printers \
		   sm_printer.add%bos.rte.printers

.include <../subsys.mk>
.include <packdep.mk>
.include <${RULES_MK}>
