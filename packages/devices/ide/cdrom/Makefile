# @(#)36        1.2  src/packages/devices/ide/cdrom/Makefile, pkgdevbase, pkg41J, 9510B 2/21/95 14:27:16
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
VPATH           := ${VPATH}:${MAKETOP}/rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}/rspc/kernext/idecdrom
VPATH		:= ${VPATH}:${MAKETOP}/rspc/kernext/idecdrom/root

BFF             = devices.ide.cdrom

ILIST           = ${BFF}
IDIR            = /

OPTIONS         = devices.ide.cdrom.rte

ROOT_OPTIONS    = devices.ide.cdrom.rte

.include <rte/packdep.mk>

.include <${RULES_MK}>
