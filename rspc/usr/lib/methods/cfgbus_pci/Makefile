# @(#)11        1.2  src/rspc/usr/lib/methods/cfgbus_pci/Makefile, rspccfg, rspc41J, 9513A_all 3/28/95 16:07:54
#
# COMPONENT_NAME: rspccfg
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ${MAKETOP}/bos/usr/lib/methods/common:${MAKETOP}/bos/usr/lib/methods/graphics

PROGRAMS	= cfgbus_pci

OFILES		= bcm.o cfgbus_pci.o sync.o adaptcfg.o 

ILIST		= cfgbus_pci

IDIR		= /usr/lib/methods/

INCFLAGS	+= -I${MAKETOP}/bos/usr/lib/methods/common	\
        	   -I${MAKETOP}/bos/usr/lib/methods/cfgbus	

LIBS		= -lodm -lcfg 

.include <${RULES_MK}>


