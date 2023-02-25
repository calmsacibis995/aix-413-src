# @(#)52	1.3  src/packages/X11/x_st_mgr/Makefile, pkgxstmgr, pkg411, GOLD410 6/27/94 08:13:20
#
#   COMPONENT_NAME: pkgxstmgr
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= compat
VPATH		:=${VPATH}:${MAKETOP}/x_st_mgr/stanza
VPATH		:=${VPATH}:rte
VPATH		:=${VPATH}:rte/root

BFF		= X11.x_st_mgr

ILIST		= ${BFF}
IDIR		= /

ROOT_OPTIONS	= X11.x_st_mgr.rte

OPTIONS		= X11.x_st_mgr.rte \
		  X11.x_st_mgr.compat



.include <rte/packdep.mk>
.include <compat/packdep.mk>
.include <${RULES_MK}>
