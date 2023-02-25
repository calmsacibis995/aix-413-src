# @(#)16        1.2  src/rspc/usr/lib/methods/cfgbus_isa/Makefile, rspccfg, rspc41J, 9513A_all 3/28/95 15:56:16
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
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH		= ../cfgbus_pci

#CC_OPT_LEVEL =
#CC_OPT_LEVEL =-DCFGDEBUG
#CC_OPT_LEVEL = -g -DCFGDEBUG
#STRIP_FLAG =

PROGRAMS	= cfgbus_isa

OFILES          = bcm.o cfgbus_isa.o adaptcfg.o \
		  cfgisa_map.o \
		  cfgisa_sup.o \
		  cfgisa_pddv.o 

ILIST		= cfgbus_isa

IDIR		= /usr/lib/methods/

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/cfgbus \
		  -I${MAKETOP}/bos/usr/lib/methods/common

LIBS		= -lodm -lcfg

.include <${RULES_MK}>


