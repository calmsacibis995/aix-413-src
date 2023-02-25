# @(#)93        1.7  src/packages/devices/scsi/tape/Makefile, pkgdevbase, pkg411, GOLD410 6/30/94 16:14:56
#
# COMPONENT_NAME: pkgdevbase
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		 = diag:rte:rspc
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass/

BFF		= devices.scsi.tape

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.scsi.tape.rte \
		  devices.scsi.tape.diag \
		  devices.scsi.tape.rspc


.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <rspc/packdep.mk>

.include <${RULES_MK}>
