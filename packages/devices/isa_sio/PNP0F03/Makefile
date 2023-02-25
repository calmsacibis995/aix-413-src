# @(#)77        1.1  src/packages/devices/isa_sio/PNP0F03/Makefile, pkgrspc, pkg41J, 9511A_all 3/6/95 16:57:35
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


BFF		= devices.isa_sio.PNP0F03

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.isa_sio.PNP0F03.rte \
		  devices.isa_sio.PNP0F03.diag 

.include <rte/packdep.mk>
.include <diag/packdep.mk>

.include <${RULES_MK}>
