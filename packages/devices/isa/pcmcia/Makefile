# @(#)74	1.1  src/packages/devices/isa/pcmcia/Makefile, pkgrspc, pkg411, 9434B411a 8/24/94 19:05:21
#
#   COMPONENT_NAME: pkgrspc
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

VPATH            = rte
VPATH		:= ${VPATH}:${MAKETOP}rspc/objclass
VPATH		:= ${VPATH}:${MAKETOP}rspc/kernext/pcmcia/ssisa

BFF             = devices.isa.pcmcia

ILIST		= ${BFF}
IDIR		= /

OPTIONS         = devices.isa.pcmcia.rte

ROOT_OPTIONS    = devices.isa.pcmcia.rte

.include <rte/packdep.mk>

.include <${RULES_MK}>
