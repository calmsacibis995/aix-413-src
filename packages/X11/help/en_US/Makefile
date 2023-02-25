# @(#)19	1.4  src/packages/X11/help/en_US/Makefile, pkgxcde, pkg411, GOLD410 2/22/94 15:16:28
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
VPATH		= Dt/helpmin
VPATH		:= ${VPATH}:Dt/helpinfo
VPATH		:= ${VPATH}:Dt/helpadt

BFF		= X11.help.en_US

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= X11.help.en_US.Dt.helpmin 	\
                  X11.help.en_US.Dt.helpinfo	\
                  X11.help.en_US.Dt.helpadt


.include <Dt/helpmin/packdep.mk>
.include <Dt/helpinfo/packdep.mk>
.include <Dt/helpadt/packdep.mk>


.include <${RULES_MK}>
