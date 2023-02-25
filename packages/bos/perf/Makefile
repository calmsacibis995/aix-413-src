# @(#)20        1.3  src/packages/bos/perf/Makefile, pkgperf, pkg411, GOLD410 8/16/93 11:56:54
#
# COMPONENT_NAME: pkgperf
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
VPATH		= diag_tool:pmr

BFF		= bos.perf

ILIST		= ${BFF}
IDIR		= /

OPTIONS		= bos.perf.diag_tool \
		  bos.perf.pmr

ROOT_OPTIONS	= bos.perf.diag_tool \
		  bos.perf.pmr

.include <diag_tool/packdep.mk>
.include <pmr/packdep.mk>

.include <${RULES_MK}>
