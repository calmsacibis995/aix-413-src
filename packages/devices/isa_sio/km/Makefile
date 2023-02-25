# @(#)37        1.2  src/packages/devices/isa_sio/km/Makefile, pkgrspc, pkg41J, 9510A_all 2/28/95 10:17:20
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
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = rte:diag
VPATH	         := ${VPATH}:${MAKETOP}/rspc/objclass
VPATH	         := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}


BFF		= devices.isa_sio.km

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.isa_sio.km.rte \
                  devices.isa_sio.km.diag

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
