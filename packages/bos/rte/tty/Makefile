# @(#)11        1.12  src/packages/bos/rte/tty/Makefile, pkgbossub, pkg41J, 9523A_all 6/5/95 16:09:32
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

OPTIONS		= bos.rte.tty

ROOT_OPTIONS	= bos.rte.tty

FILESET_BASE	= tty
SCRIPTS		= bos.rte.tty.add
OTHERS		= swvpd.make swvpdroot.make sysck.make

USR_ODMADD_LIST += cfgrule.pty.add%bos.rte.tty \
		   sm_pty.add%bos.rte.tty \
		   sm_tty.add%bos.rte.tty

ROOT_ODMADD_LIST += pty.add%bos.rte.tty

.include <../subsys.mk>
.include <packdep.mk>
.include <${RULES_MK}>
