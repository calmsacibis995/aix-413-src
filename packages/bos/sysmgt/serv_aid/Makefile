# @(#)93        1.7  src/packages/bos/sysmgt/serv_aid/Makefile, pkgserv_aid, pkg41B, 9505A 1/24/95 09:08:01
#
#   COMPONENT_NAME: PKGSERV_AID
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		 = ${MAKETOP}/bos/usr/bin/errlg:${MAKETOP}/bos/usr/bin/sysdumpdev
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/errlg/odm:root


BFF		= bos.sysmgt

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.sysmgt.serv_aid
ROOT_OPTIONS	= bos.sysmgt.serv_aid

.include <packdep.mk>

.include <${RULES_MK}>

root/bos.sysmgt.serv_aid.err.S: $${.TARGET:T}
		-${MKDIR} root
		${CP} ${.ALLSRC} ${.TARGET}
