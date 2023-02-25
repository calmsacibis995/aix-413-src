# @(#)52        1.1  src/packages/bos/powermgt/rte/Makefile, pkgpwrmgt, pkg411, 9435A411a 8/29/94 08:23:45
#
# COMPONENT_NAME: pkgpwrmgt
#
# FUNCTIONS:
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

# We need to visit each of the subsystem directories.  These
# directories are defined in subsystem.mk.  DO NOT ADD ANY
# MORE subsystem directories to this SUBDIRS list here.  Note
# that the tracking of lpp_id has also been moved to subsys.mk

VPATH		= root
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:${MAKETOP}/rspc/objclass

BFF		= bos.powermgt
ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.powermgt.rte
ROOT_OPTIONS	= bos.powermgt.rte

.include <packdep.mk>

.include <${RULES_MK}>
