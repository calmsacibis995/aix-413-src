# @(#)00        1.18  src/packages/bos/net/Makefile, pkgnet, pkg411, 9432B411a 8/9/94 12:05:13
#
# COMPONENT_NAME: pkgnet
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

# The following line was added by feature 140116 to create the client
# BFF image.  DO NOT REMOVE !!!
.if (${PTF_UPDATE} != "yes" )
SUBDIRS          = client
.endif

VPATH		 = ate
VPATH		:= ${VPATH}:ncs:ncs/root
VPATH		:= ${VPATH}:nfs/server:nfs/client:nfs/client/root:nfs/adt
VPATH		:= ${VPATH}:nis/server:nis/server/root:nis/client
VPATH		:= ${VPATH}:nis/client/root
VPATH		:= ${VPATH}:tcp/server:tcp/server/root:tcp/client
VPATH		:= ${VPATH}:tcp/client/root:tcp/smit:tcp/adt
VPATH		:= ${VPATH}:uucp:uucp/root
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/nfs/objclass
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/tcpip/usr/lpp/tcpip

BFF		= bos.net

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.net.ate \
		  bos.net.ncs \
		  bos.net.nfs.client \
		  bos.net.nfs.server \
		  bos.net.nfs.adt \
		  bos.net.nis.client \
		  bos.net.nis.server \
		  bos.net.tcp.client \
		  bos.net.tcp.server \
		  bos.net.tcp.adt \
		  bos.net.tcp.smit \
		  bos.net.uucp

ROOT_OPTIONS	= bos.net.nfs.client \
		  bos.net.nis.client \
		  bos.net.nis.server \
		  bos.net.ncs \
		  bos.net.tcp.client \
		  bos.net.tcp.server \
		  bos.net.uucp

.include <ate/packdep.mk>
.include <ncs/packdep.mk>
.include <nfs/adt/packdep.mk>
.include <nfs/client/packdep.mk>
.include <nfs/server/packdep.mk>
.include <nis/client/packdep.mk>
.include <nis/server/packdep.mk>
.include <tcp/adt/packdep.mk>
.include <tcp/client/packdep.mk>
.include <tcp/server/packdep.mk>
.include <tcp/smit/packdep.mk>
.include <uucp/packdep.mk>

.include <${RULES_MK}>
