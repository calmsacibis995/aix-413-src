# @(#)83  1.2  src/bos/usr/bin/rmslip/Makefile, rcs, bos411, 9436B411a 9/6/94 10:36:36
# 
# COMPONENT_NAME: (RCS) Remote Customer Support and Services
#
# FUNCTIONS: none
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

PROGRAMS	= rmslip

OFILES		=  rmslip.o slips.o

VPATH		= ${MAKETOP}/bos/objclass

MSGHDRS		= rmslip_msg.h

HFILES		= ${MSGHDRS} 

CATFILES	=  rmslip.cat

LIBS		= -lodm 

INCFLAGS	= 

ILIST		= ${PROGRAMS}

IDIR		= /usr/bin/

.include <${RULES_MK}>
