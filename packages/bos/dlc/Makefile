# @(#)11        1.8  src/packages/bos/dlc/Makefile, pkgdlc, pkg411, GOLD410 7/6/94 08:25:21
#
# COMPONENT_NAME: pkgdlc
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = com:com/root
VPATH		:= ${VPATH}:com_enet
VPATH		:= ${VPATH}:8023:8023/root
VPATH		:= ${VPATH}:ether:ether/root
VPATH		:= ${VPATH}:fddi:fddi/root
VPATH		:= ${VPATH}:sdlc:sdlc/root
VPATH		:= ${VPATH}:token:token/root
VPATH		:= ${VPATH}:qllc:qllc/root
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass

BFF		= bos.dlc

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.dlc.com \
		  bos.dlc.com_enet \
		  bos.dlc.8023 \
		  bos.dlc.ether \
		  bos.dlc.fddi \
		  bos.dlc.sdlc \
		  bos.dlc.qllc \
		  bos.dlc.token

ROOT_OPTIONS	= bos.dlc.com  \
		  bos.dlc.8023 \
		  bos.dlc.ether \
		  bos.dlc.fddi \
		  bos.dlc.sdlc \
		  bos.dlc.qllc \
		  bos.dlc.token

.include <com/packdep.mk>
.include <com_enet/packdep.mk>
.include <8023/packdep.mk>
.include <ether/packdep.mk>
.include <fddi/packdep.mk>
.include <sdlc/packdep.mk>
.include <qllc/packdep.mk>
.include <token/packdep.mk>

.include <${RULES_MK}>

