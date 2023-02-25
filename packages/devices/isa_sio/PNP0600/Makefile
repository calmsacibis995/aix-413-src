# @(#)54        1.1  src/packages/devices/isa_sio/PNP0600/Makefile, pkgdevbase, pkg41J, 9510B 2/16/95 15:56:51
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
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH            = com:rte
VPATH		:= ${VPATH}:${MAKETOP}/obj/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}/rspc/kernext/isa/ide
VPATH		:= ${VPATH}:${MAKETOP}/rspc/kernext/isa/ide/root

BFF             = devices.isa_sio.PNP0600

ILIST           = ${BFF}
IDIR            = /

OPTIONS         = devices.isa_sio.PNP0600.com \
                  devices.isa_sio.PNP0600.rte

ROOT_OPTIONS    = devices.isa_sio.PNP0600.rte

.include <com/packdep.mk>
.include <rte/packdep.mk>

.include <${RULES_MK}>
