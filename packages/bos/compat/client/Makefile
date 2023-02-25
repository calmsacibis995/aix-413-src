# @(#)94	1.1  src/packages/bos/compat/client/Makefile, pkg, pkg41B, 9505A 1/23/95 09:08:39
#
# COMPONENT_NAME: pkgcompat
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

PTF_UPDATE	 = no

VPATH		 = ../cmds
VPATH		:= ${VPATH}:../imk
VPATH		:= ${VPATH}:../libs
VPATH		:= ${VPATH}:../links
VPATH		:= ${VPATH}:../lan:${MAKETOP}bos/objclass/${TARGET_MACHINE}
VPATH		:= ${VPATH}:../msg
VPATH		:= ${VPATH}:../net
VPATH		:= ${VPATH}:../termcap

BFF		= bos.compat

ILIST		= ${BFF}.client
IDIR		= /

OPTIONS		= bos.compat.cmds \
		  bos.compat.imk \
		  bos.compat.libs \
		  bos.compat.links \
		  bos.compat.lan \
		  bos.compat.msg \
		  bos.compat.net \
		  bos.compat.termcap

ROOT_OPTIONS	= bos.compat.links \
		  bos.compat.termcap

.include <../cmds/packdep.mk>
.include <../imk/packdep.mk>
.include <../libs/packdep.mk>
.include <../links/packdep.mk>
.include <../lan/packdep.mk>
.include <../msg/packdep.mk>
.include <../net/packdep.mk>
.include <../termcap/packdep.mk>

.include <${RULES_MK}>

bos.compat.client : bos.compat
	${CP} bos.compat bos.compat.client
