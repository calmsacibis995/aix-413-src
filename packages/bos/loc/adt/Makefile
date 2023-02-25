# @(#)74        1.2  src/packages/bos/loc/adt/Makefile, pkglocadt, pkg411, GOLD410 1/31/94 08:26:46
#
# COMPONENT_NAME: pkglocadt
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= iconv:imk:locale:unicode

BFF		= bos.loc.adt

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= \
   bos.loc.adt.iconv \
   bos.loc.adt.imk   \
   bos.loc.adt.locale \
   bos.loc.adt.unicode

.include <iconv/packdep.mk>
.include <imk/packdep.mk>
.include <locale/packdep.mk>
.include <unicode/packdep.mk>

.include <${RULES_MK}>

