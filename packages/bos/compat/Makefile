# @(#)71	1.15  src/packages/bos/compat/Makefile, pkgcompat, pkg41J, 9519A_all 5/5/95 11:22:35
#
# COMPONENT_NAME: pkgcompat
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

# The following line was added to create the client BFF image.
# DO NOT REMOVE !!!
.if (${PTF_UPDATE} != "yes" )
SUBDIRS		 = client
.endif

VPATH		 = cmds
VPATH		:= ${VPATH}:imk
VPATH		:= ${VPATH}:libs
VPATH		:= ${VPATH}:links
VPATH		:= ${VPATH}:lan:${MAKETOP}bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:msg
VPATH		:= ${VPATH}:net
VPATH		:= ${VPATH}:NetInstl
VPATH		:= ${VPATH}:termcap:termcap/data

BFF		= bos.compat

ILIST		= ${BFF} ${BFF}.data
IDIR		= /

OPTIONS		= bos.compat.cmds \
		  bos.compat.imk \
		  bos.compat.libs \
		  bos.compat.links \
		  bos.compat.lan \
		  bos.compat.msg \
		  bos.compat.NetInstl \
		  bos.compat.net \
		  bos.compat.termcap

ROOT_OPTIONS	= bos.compat.links \
		  bos.compat.net \
		  bos.compat.NetInstl \
		  bos.compat.termcap

SHARE_OPTIONS   = bos.compat.termcap.data

.include <cmds/packdep.mk>
.include <imk/packdep.mk>
.include <libs/packdep.mk>
.include <links/packdep.mk>
.include <lan/packdep.mk>
.include <msg/packdep.mk>
.include <net/packdep.mk>
.include <NetInstl/packdep.mk>
.include <termcap/packdep.mk>
.include <termcap/data/packdep.mk>

.include <${RULES_MK}>
