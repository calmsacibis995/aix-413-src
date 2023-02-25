# @(#)92        1.7  src/packages/devices/mca/8f70/diag/Makefile, pkgdevdiag, pkg41J, 9519B_all 5/11/95 14:12:04
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
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
PTF_UPDATE	= no

VPATH           = ${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.8f70

ILIST		= ${BFF}.client
IDIR		= /

OPTIONS		= devices.mca.8f70.diag

.include <packdep.mk>

.include <${RULES_MK}>

devices.mca.8f70.client: devices.mca.8f70
		${CP} devices.mca.8f70 devices.mca.8f70.client
