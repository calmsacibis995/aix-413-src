# @(#)87        1.2  src/packages/devices/isa/c1x/Makefile, pkgrspc, pkg41J, 9513A_all 3/22/95 14:56:09
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
VPATH            = rte:diag
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass:${MAKETOP}/rspc/usr/lib/methods
VPATH           := ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF             = devices.isa.c1x

ILIST           = ${BFF}
IDIR            = /

OPTIONS         = devices.isa.c1x.rte \
	          devices.isa.c1x.diag

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
