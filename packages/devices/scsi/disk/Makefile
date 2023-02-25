# @(#)85	1.10  src/packages/devices/scsi/disk/Makefile, pkgdevbase, pkg411, GOLD410 6/9/94 16:38:31
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
VPATH		 = diag/rte:diag/com:rte:rspc
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass/


BFF             = devices.scsi.disk

ILIST           = ${BFF}
IDIR            = /

OPTIONS         = devices.scsi.disk.diag.rte \
		  devices.scsi.disk.diag.com \
		  devices.scsi.disk.rte \
		  devices.scsi.disk.rspc



.include <diag/rte/packdep.mk>
.include <diag/com/packdep.mk>
.include <rte/packdep.mk>
.include <rspc/packdep.mk>

.include <${RULES_MK}>
