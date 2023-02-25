# @(#)92        1.2  src/packages/devices/isa_sio/PNP0401/Makefile, pkgrspc, pkg41J, 9520A_all 5/16/95 09:09:30
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
VPATH            := ${VPATH}:${MAKETOP}/rspc/objclass
VPATH            := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}


BFF		= devices.isa_sio.PNP0401

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.isa_sio.PNP0401.rte \
		  devices.isa_sio.PNP0401.diag 

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
