# @(#)95        1.3  src/packages/bos/sysmgt/smit/Makefile, pkgsmit, pkg411, 9439A411a 9/26/94 18:09:01
#
# COMPONENT_NAME: pkgsmit
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
SUBDIRS		= softlic

# It is important that the .if and .endif in this Makefile
# not change inorder for the update process to work correctly.

.if defined(IN_SANDBOX)

VPATH		= ${MAKETOP}/bos/objclass:${MAKETOP}/bos/objclass/${TARGET_MACHINE}

BFF		= bos.sysmgt

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.sysmgt.smit

.include <packdep.mk>

.endif

.include <${RULES_MK}>
