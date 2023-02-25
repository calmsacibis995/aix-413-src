# @(#)53        1.2  src/packages/bos/net/client/Makefile, pkg, pkg41J, 9519B_all 5/11/95 14:08:04
#
# COMPONENT_NAME: pkg
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PTF_UPDATE	 = no

VPATH		 = ../ate
VPATH		:= ${VPATH}:../ncs:../ncs/root
VPATH		:= ${VPATH}:../nfs/client:../nfs/client/root
VPATH		:= ${VPATH}:../nfs/server:../nfs/adt
VPATH		:= ${VPATH}:../nis/client:../nis/client/root
VPATH		:= ${VPATH}:../tcp/client:../tcp/client/root
VPATH		:= ${VPATH}:../tcp/server:../tcp/server/root
VPATH		:= ${VPATH}:../tcp/smit:../tcp/adt
VPATH		:= ${VPATH}:../uucp:../uucp/root
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/nfs/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/tcpip/usr/lpp/tcpip

BFF		= bos.net

ILIST		= ${BFF}.client
IDIR		= /

OPTIONS		= bos.net.ate \
		  bos.net.ncs \
		  bos.net.nfs.client \
		  bos.net.nfs.server \
		  bos.net.nfs.adt \
		  bos.net.nis.client \
		  bos.net.tcp.server \
		  bos.net.tcp.client \
		  bos.net.tcp.adt \
		  bos.net.tcp.smit \
		  bos.net.uucp

ROOT_OPTIONS	= bos.net.nfs.client \
		  bos.net.nis.client \
		  bos.net.ncs \
		  bos.net.tcp.client \
		  bos.net.tcp.server \
		  bos.net.uucp

.include <../ate/packdep.mk>
.include <../ncs/packdep.mk>
.include <../nfs/adt/packdep.mk>
.include <../nfs/client/packdep.mk>
.include <../nfs/server/packdep.mk>
.include <../nis/client/packdep.mk>
.include <../tcp/adt/packdep.mk>
.include <../tcp/client/packdep.mk>
.include <../tcp/server/packdep.mk>
.include <../tcp/smit/packdep.mk>
.include <../uucp/packdep.mk>

.include <${RULES_MK}>

bos.net.client : bos.net
	${CP} bos.net bos.net.client
