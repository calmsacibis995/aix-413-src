# @(#)09        1.3  src/packages/bos/net/nis/Makefile, pkgnis, pkg411, GOLD410 2/11/94 10:54:19
#
# COMPONENT_NAME: pkgnis
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= client:client/root:server:server/root

BFF		= bos.net

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.net.nis.client bos.net.nis.server

ROOT_OPTIONS	= bos.net.nis.client bos.net.nis.server

.include <client/packdep.mk>
.include <server/packdep.mk>

.include <${RULES_MK}>

