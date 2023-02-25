# @(#)45        1.16  src/packages/bos/rte/Makefile, pkgbos, pkg411, GOLD410 6/15/94 16:59:35
#
# COMPONENT_NAME: pkgbos
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# We need to visit each of the subsystem directories.  These
# directories are defined in subsystem.mk.  DO NOT ADD ANY
# MORE subsystem directories to this SUBDIRS list here.  Note
# that the tracking of lpp_id has also been moved to subsys.mk

.include <subsys.mk>
SUBDIRS = etc usr up mp ${SUBSYSTEMS}

VPATH		= root

BFF		= bos

OPTIONS		= bos.rte
ROOT_OPTIONS	= bos.rte

SCRIPTS		= bos.rte.first

LINKFLAG	= -L
PTF_UPDATE      = no
ADEPACKAGEFLAGS = -F bos.rte.first
LPPMEDIATYPE	= O

ILIST		= ${BFF}
IDIR		= /

.include <packdep.mk>

.include <${RULES_MK}>
