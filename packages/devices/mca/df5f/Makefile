# @(#)13        1.5  src/packages/devices/mca/df5f/Makefile, pkgdevbase, pkg411, GOLD410 2/22/94 16:11:43
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

VPATH		 = com:rte
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= devices.mca.df5f

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.mca.df5f.rte \
		  devices.mca.df5f.com 


.include <com/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
