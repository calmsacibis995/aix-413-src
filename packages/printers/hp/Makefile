# @(#)12 1.4 src/packages/printers/hp/Makefile, pkgdevprint, pkg41J, 9512A_all 3/15/95 18:51:29
#
# COMPONENT_NAME: pkgdevprint
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = ${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:../hpJetDirect:../hpJetDirect/attach
VPATH		:= ${VPATH}:../hplj-2:../hplj-2/rte
VPATH		:= ${VPATH}:../hplj-2p_CN/rte
VPATH		:= ${VPATH}:../hplj-3:../hplj-3/rte
VPATH		:= ${VPATH}:../hplj-3si:../hplj-3si/rte
VPATH		:= ${VPATH}:../hplj-4:../hplj-4/rte
VPATH		:= ${VPATH}:../hplj-4si:../hplj-4si/rte
VPATH		:= ${VPATH}:../hplj-4+:../hplj-4+/rte
VPATH		:= ${VPATH}:../hplj-4v:../hplj-4v/rte
VPATH		:= ${VPATH}:../hplj-c:../hplj-c/rte

BFF		= printers

ILIST		= ${BFF}.hp
IDIR		= /

OPTIONS		= printers.hpJetDirect.attach \
		  printers.hplj-2.rte \
		  printers.hplj-2p_CN.rte \
		  printers.hplj-3.rte \
		  printers.hplj-3si.rte \
		  printers.hplj-4.rte \
		  printers.hplj-4si.rte \
		  printers.hplj-4+.rte \
		  printers.hplj-4v.rte \
		  printers.hplj-c.rte

.include <../hpJetDirect/attach/packdep.mk>
.include <../hplj-2/rte/packdep.mk>
.include <../hplj-2p_CN/rte/packdep.mk>
.include <../hplj-3/rte/packdep.mk>
.include <../hplj-3si/rte/packdep.mk>
.include <../hplj-4/rte/packdep.mk>
.include <../hplj-4si/rte/packdep.mk>
.include <../hplj-4+/rte/packdep.mk>
.include <../hplj-4v/rte/packdep.mk>
.include <../hplj-c/rte/packdep.mk>

.include <${RULES_MK}>

printers.hp : printers
	${CP} printers printers.hp
