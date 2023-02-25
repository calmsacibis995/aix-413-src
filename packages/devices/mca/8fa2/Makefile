# @(#)63        1.5  src/packages/devices/mca/8fa2/Makefile, pkgdevcommo, pkg411, 9434B411a 8/24/94 09:27:18
#
# COMPONENT_NAME: pkgdevcommo
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
VPATH            = rte:diag
VPATH           := $(VPATH):${MAKETOP}/bos/objclass/${TARGET_MACHINE}
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/mps_tok
VPATH           := ${VPATH}:${MAKETOP}/bos/kernext/mps_tok/root

BFF		= devices.mca.8fa2

ILIST		= ${BFF}
IDIR		= /

OPTIONS         = devices.mca.8fa2.rte \
		  devices.mca.8fa2.diag

ROOT_OPTIONS    = devices.mca.8fa2.rte

.include <diag/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
