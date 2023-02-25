# @(#)13        1.3  src/packages/devices/serial/gio/Makefile, pkgdevgraphics, pkg41J, 9510A_all 2/28/95 17:56:20
#
# COMPONENT_NAME: pkgdevtty
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		 = rte:diag:X11
VPATH		:= ${VPATH}:${MAKETOP}/bos/objclass

BFF		= devices.serial.gio

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= devices.serial.gio.rte	\
			  devices.serial.gio.diag	\
			  devices.serial.gio.X11

ROOT_OPTIONS = devices.serial.gio.rte

.include <rte/packdep.mk>
.include <diag/packdep.mk>
.include <X11/packdep.mk>

.include <${RULES_MK}>
