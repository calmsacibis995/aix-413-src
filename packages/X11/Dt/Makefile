# @(#)15	1.12  src/packages/X11/Dt/Makefile, pkgxcde, pkg41J, 9514A_all 4/4/95 18:46:32
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# X11.Dt Makefile
#
VPATH		= rte
VPATH		:= ${VPATH}:bitmaps
VPATH		:= ${VPATH}:lib
VPATH		:= ${VPATH}:helprun
VPATH		:= ${VPATH}:helpmin
VPATH		:= ${VPATH}:helpinfo
VPATH		:= ${VPATH}:ToolTalk
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass
VPATH		:= ${VPATH}:adt
VPATH		:= ${VPATH}:xdt2cde
VPATH		:= ${VPATH}:compat

BFF		= X11.Dt

ILIST		= ${BFF}

IDIR		= /

OPTIONS		= X11.Dt.ToolTalk\
                  X11.Dt.lib \
		  X11.Dt.helpmin \
		  X11.Dt.helprun \
                  X11.Dt.rte \
                  X11.Dt.bitmaps \
		  X11.Dt.helpinfo \
		  X11.Dt.xdt2cde \
		  X11.Dt.adt	\
		  X11.Dt.compat

ROOT_OPTIONS	= X11.Dt.rte	\
		  X11.Dt.helpmin	\
		  X11.Dt.helpinfo

.include <rte/packdep.mk>
.include <bitmaps/packdep.mk>
.include <lib/packdep.mk>
.include <helprun/packdep.mk>
.include <helpmin/packdep.mk>
.include <helpinfo/packdep.mk>
.include <ToolTalk/packdep.mk>
.include <xdt2cde/packdep.mk>
.include <adt/packdep.mk>
.include <compat/packdep.mk>


.include <${RULES_MK}>
