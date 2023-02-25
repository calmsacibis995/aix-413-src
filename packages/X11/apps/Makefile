# @(#)50	1.8  src/packages/X11/apps/Makefile, pkgx, pkg41J, 9512A_all 3/21/95 00:59:32
#
#   COMPONENT_NAME:	pkgx
#
#   FUNCTIONS:		Makefile
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 =	clients
VPATH		:=	${VPATH}:config
VPATH		:=	${VPATH}:custom
VPATH		:=	${VPATH}:ims
VPATH		:=	${VPATH}:msmit
VPATH		:=	${VPATH}:rte
VPATH		:=	${VPATH}:aixterm
VPATH		:=	${VPATH}:xdm
VPATH		:=	${VPATH}:xterm
VPATH		:=	${VPATH}:util
VPATH		:=	${VPATH}:pcmcia
VPATH		:=	${VPATH}:pm

BFF		 =	X11.apps

ILIST		 =	${BFF}
IDIR		 =	/

OPTIONS		 =	X11.apps.clients	\
			X11.apps.config		\
			X11.apps.custom		\
			X11.apps.ims		\
			X11.apps.msmit		\
			X11.apps.rte		\
			X11.apps.aixterm	\
			X11.apps.xdm		\
			X11.apps.xterm		\
			X11.apps.util		\
			X11.apps.pcmcia		\
			X11.apps.pm

.include <clients/packdep.mk>
.include <config/packdep.mk>
.include <custom/packdep.mk>
.include <ims/packdep.mk>
.include <msmit/packdep.mk>
.include <rte/packdep.mk>
.include <aixterm/packdep.mk>
.include <xdm/packdep.mk>
.include <xterm/packdep.mk>
.include <util/packdep.mk>
.include <pcmcia/packdep.mk>
.include <pm/packdep.mk>

.include <${RULES_MK}>
