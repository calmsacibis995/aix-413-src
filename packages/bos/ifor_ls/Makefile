# @(#)78	1.3  src/packages/bos/ifor_ls/Makefile, pkgnetls, pkg411, GOLD410 6/9/94 13:11:30
#
#   COMPONENT_NAME: pkgnetls
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
VPATH		 = server:client
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/bin/netls

BFF		= bos.ifor_ls

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.ifor_ls.server \
		  bos.ifor_ls.client

ROOT_OPTIONS	= bos.ifor_ls.server

.include <server/packdep.mk>
.include <client/packdep.mk>

.include <${RULES_MK}>
