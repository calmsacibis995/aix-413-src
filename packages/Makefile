# @(#)53        1.31.1.9  src/packages/Makefile, pkg, pkg41J, 9524E_all 6/14/95 20:09:31
#
# COMPONENT_NAME: pkg
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS = bos devices printers

.if exists (${MAKETOP}/des)
SUBDIRS += des
.endif

.if exists (${MAKETOP}/gos)
SUBDIRS += X11
.endif

SUBDIRS += dps    \
	  uimx     ipx \
	    stacklist internet_server 

# LPPs taken out of 41J build:
#    blkmux 
#


.include <${RULES_MK}>
