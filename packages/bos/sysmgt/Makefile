# @(#)22        1.24  src/packages/bos/sysmgt/Makefile, pkgsysmgt, pkg41B, 9505A 1/24/95 09:04:53
#
# COMPONENT_NAME: pkgsysmgt
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

# The following line has been added for feature 140116 to enable the 
# client media.  DO NOT REMOVE THIS LINE!!!
.if (${PTF_UPDATE} != "yes" )
SUBDIRS		= client smit
.else
SUBDIRS		= smit
.endif

VPATH		 = smit
VPATH		:= ${VPATH}:serv_aid:serv_aid/root
VPATH		:= ${VPATH}:loginlic:loginlic/root
VPATH		:= ${VPATH}:trace:trace/root
VPATH		:= ${VPATH}:sysbr
VPATH		:= ${VPATH}:nim/client:nim/spot:nim/master
VPATH		:= ${VPATH}:quota
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/errlg
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/errlg/odm
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/sysdumpdev
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/sbin/trace
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/trcrpt
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/nim/objrepos

BFF		= bos.sysmgt

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.sysmgt.loginlic \
		  bos.sysmgt.smit \
		  bos.sysmgt.trace \
		  bos.sysmgt.sysbr \
		  bos.sysmgt.serv_aid \
		  bos.sysmgt.quota \
		  bos.sysmgt.nim.client \
		  bos.sysmgt.nim.spot \
		  bos.sysmgt.nim.master

ROOT_OPTIONS	= bos.sysmgt.loginlic \
		  bos.sysmgt.trace \
		  bos.sysmgt.serv_aid \
		  bos.sysmgt.nim.client \
		  bos.sysmgt.nim.master \
		  bos.sysmgt.sysbr

.include <smit/packdep.mk>
.include <sysbr/packdep.mk>
.include <serv_aid/packdep.mk>
.include <trace/packdep.mk>
.include <quota/packdep.mk>
.include <loginlic/packdep.mk>
.include <nim/client/packdep.mk>
.include <nim/spot/packdep.mk>
.include <nim/master/packdep.mk>

.include <${RULES_MK}>

root/bos.sysmgt.serv_aid.err.S: $${.TARGET:T}
		-${MKDIR} root
		${CP} ${.ALLSRC} ${.TARGET}

root/bos.sysmgt.trace.trc.S: $${.TARGET:T}
		-${MKDIR} root
		${CP} ${.ALLSRC} ${.TARGET}
