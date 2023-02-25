# @(#)45        1.1  src/packages/devices/ide/disk/Makefile, pkgdevbase, pkg41J, 9510B 2/16/95 15:56:22
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
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= rte
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}/rspc/kernext/idedisk
VPATH		:= ${VPATH}:${MAKETOP}/rspc/kernext/idedisk/root

BFF             = devices.ide.disk

ILIST           = ${BFF}
IDIR            = /

OPTIONS         = devices.ide.disk.rte

ROOT_OPTIONS	= devices.ide.disk.rte

.include <rte/packdep.mk>

.include <${RULES_MK}>

