# @(#)28	1.1  src/bldenv/pkgtools/lookupfixdata/Makefile, pkgtools, bos412, GOLDA411a 6/22/94 16:49:55
#
#   COMPONENT_NAME: PKGTOOLS
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS		= lookupfixdata
INCFLAGS		+= -I${MAKETOP}bldenv/pkgtools/common
COMMON_OFILES		= ../common/util.o
lookupfixdata_OFILES	= lookupfixdata.o ${COMMON_OFILES}

ILIST           	= ${PROGRAMS}
IDIR            	= /usr/bin/

.include <${RULES_MK}>
