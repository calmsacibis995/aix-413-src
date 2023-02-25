# @(#)37	1.1  src/rspc/usr/lbin/ientdd/Makefile, isaent, rspc411, 9435A411a 8/29/94 16:36:14
#
# COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
#
# FUNCTIONS: Makefile for Device Driver Dump Formatter
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994, 
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS        = ientdd 

VPATH           = ${MAKETOP}/rspc/kernext/isa/ient

ientdd_OFILES	= main.o isaent_dmp.o 

INCFLAGS	= -I${MAKETOP}/rspc/kernext/isa/ient

ILIST           = ientdd 

ientdd_IDIR	= /usr/lib/ras/dmprtns/

.include <${RULES_MK}>
