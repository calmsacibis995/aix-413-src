# @(#)20	1.7  src/packages/X11/help/en_US/Dt/Makefile, pkgxcde, pkg411, GOLD410 4/14/94 16:12:14
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: Makefile
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
# X11.help.en_US.Dt Makefile
#
VPATH		= helpmin
VPATH		:= ${VPATH}:helpinfo
#VPATH		:= ${VPATH}:helpadt

BFF		= X11.help.en_US

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= X11.help.en_US.Dt.helpmin \
                  X11.help.en_US.Dt.helpinfo

#                  X11.help.en_US.Dt.helpinfo\
#                  X11.help.en_US.Dt.helpadt


.include <helpmin/packdep.mk>
.include <helpinfo/packdep.mk>
#.include <helpadt/packdep.mk>


.include <${RULES_MK}>
