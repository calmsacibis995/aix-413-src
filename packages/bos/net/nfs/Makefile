# @(#)05        1.5  src/packages/bos/net/nfs/Makefile, pkgnfs, pkg411, GOLD410 2/23/94 13:54:40
#
# COMPONENT_NAME: pkgnfs
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

VPATH		 = client:client/root:server:adt
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/nfs/objclass

BFF		= bos.net

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.net.nfs.client \
		  bos.net.nfs.server \
		  bos.net.nfs.adt

ROOT_OPTIONS	= bos.net.nfs.client

.include <adt/packdep.mk>
.include <client/packdep.mk>
.include <server/packdep.mk>

.include <${RULES_MK}>
