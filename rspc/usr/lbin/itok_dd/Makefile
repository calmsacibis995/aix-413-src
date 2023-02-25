# @(#)34	1.1  src/rspc/usr/lbin/itok_dd/Makefile, isatok, rspc411, 9435A411a 8/29/94 16:32:06
#
# COMPONENT_NAME: isatok - IBM ISA Tokenring Device Driver
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
PROGRAMS        = itok_dd 

VPATH           = ${MAKETOP}/rspc/kernext/isa/itok

itok_dd_OFILES	= main.o isatok_dmp.o 

INCFLAGS	= -I${MAKETOP}/rspc/kernext/isa/itok

ILIST           = itok_dd 

itok_dd_IDIR	= /usr/lib/ras/dmprtns/

.include <${RULES_MK}>
