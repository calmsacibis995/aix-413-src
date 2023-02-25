# @(#)54        1.5  src/packages/bos/sysmgt/client/Makefile, pkg, pkg41B, 9504A 1/18/95 08:46:37
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
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PTF_UPDATE	 = no

VPATH		 = ../smit:../sysbr
VPATH		:= ${VPATH}:../serv_aid:../serv_aid/root
VPATH		:= ${VPATH}:../trace:../trace/root
VPATH		:= ${VPATH}:../nim/client:../nim/spot
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/errlg
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/sysdumpdev
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/sbin/trace
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/trcrpt
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/errlg/odm

BFF		= bos.sysmgt

ILIST		= ${BFF}.client
IDIR		= /

OPTIONS		= bos.sysmgt.smit \
		  bos.sysmgt.trace \
		  bos.sysmgt.sysbr \
		  bos.sysmgt.serv_aid \
		  bos.sysmgt.nim.client \
		  bos.sysmgt.nim.spot

ROOT_OPTIONS	= bos.sysmgt.trace \
		  bos.sysmgt.serv_aid \
		  bos.sysmgt.nim.client \
		  bos.sysmgt.sysbr

.include <../smit/packdep.mk>
.include <../sysbr/packdep.mk>
.include <../serv_aid/packdep.mk>
.include <../trace/packdep.mk>
.include <../nim/client/packdep.mk>
.include <../nim/spot/packdep.mk>

.include <${RULES_MK}>

root/bos.sysmgt.serv_aid.err.S: $${.TARGET:T}
		-${MKDIR} root
		${CP} ${.ALLSRC} ${.TARGET}

root/bos.sysmgt.trace.trc.S: $${.TARGET:T}
		-${MKDIR} root
		${CP} ${.ALLSRC} ${.TARGET}

bos.sysmgt.client : bos.sysmgt
	${CP} bos.sysmgt bos.sysmgt.client
