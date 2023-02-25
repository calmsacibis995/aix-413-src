# @(#)58        1.15  src/packages/bos/adt/Makefile, pkgadt, pkg41J, 9522A_all 5/26/95 15:35:27
#
# COMPONENT_NAME: pkgadt
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# DO NOT REMOVE THE FOLLOWING LINES UNTIL 4.2
.if (${PTF_UPDATE} != "yes")
SUBDIRS		= client
.endif

VPATH		 = base
VPATH		:= ${VPATH}:data
VPATH		:= ${VPATH}:debug
VPATH		:= ${VPATH}:graphics
VPATH		:= ${VPATH}:include
VPATH		:= ${VPATH}:lib
VPATH		:= ${VPATH}:libm
VPATH		:= ${VPATH}:prof
VPATH		:= ${VPATH}:prt_tools
VPATH		:= ${VPATH}:samples
VPATH		:= ${VPATH}:sccs
VPATH		:= ${VPATH}:syscalls
VPATH		:= ${VPATH}:utils

BFF		= bos.adt

ILIST		= ${BFF} ${BFF}.data
IDIR		= /

OPTIONS		= bos.adt.base \
		  bos.adt.debug \
		  bos.adt.graphics \
		  bos.adt.include \
		  bos.adt.lib \
		  bos.adt.libm \
		  bos.adt.prof \
		  bos.adt.prt_tools \
		  bos.adt.samples \
		  bos.adt.sccs \
		  bos.adt.syscalls \
		  bos.adt.utils

SHARE_OPTIONS	= bos.adt.data

.include <base/packdep.mk>
.include <data/packdep.mk>
.include <debug/packdep.mk>
.include <graphics/packdep.mk>
.include <include/packdep.mk>
.include <lib/packdep.mk>
.include <libm/packdep.mk>
.include <prof/packdep.mk>
.include <prt_tools/packdep.mk>
.include <samples/packdep.mk>
.include <sccs/packdep.mk>
.include <syscalls/packdep.mk>
.include <utils/packdep.mk>

.include <${RULES_MK}>
