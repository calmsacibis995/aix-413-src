# @(#)13        1.6  src/packages/bos/net/tcp/Makefile, pkgtcpip, pkg411, GOLD410 2/23/94 13:57:19
#
# COMPONENT_NAME: pkgtcpip
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = client:client/root:server:server/root:smit:adt
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/tcpip/usr/lpp/tcpip

BFF		= bos.net

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.net.tcp.client \
		  bos.net.tcp.server \
		  bos.net.tcp.smit \
		  bos.net.tcp.adt

ROOT_OPTIONS	= bos.net.tcp.client bos.net.tcp.server

.include <client/packdep.mk>
.include <server/packdep.mk>
.include <smit/packdep.mk>
.include <adt/packdep.mk>

.include <${RULES_MK}>
