# @(#)35	1.5  src/packages/devices/scsi/disk/diag/Makefile, pkgdevdiag, pkg411, GOLD410 6/7/94 20:42:42
#
# COMPONENT_NAME: pkgdevdiag
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

VPATH		= com:rte
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.scsi.disk

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.scsi.disk.diag.rte \
		  devices.scsi.disk.diag.com

.include <rte/packdep.mk>
.include <com/packdep.mk>

.include <${RULES_MK}>
