# @(#)40        1.2  src/packages/devices/pci/00100300/Makefile, pkgrspc, pkg41J, 9514A_all 4/4/95 16:36:44
#
# COMPONENT_NAME: pkgrspc
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = rte:diag
VPATH	         := ${VPATH}:${MAKETOP}/rspc/objclass
VPATH	         := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}


BFF		= devices.pci.00100300

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.pci.00100300.rte \
                  devices.pci.00100300.diag

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
