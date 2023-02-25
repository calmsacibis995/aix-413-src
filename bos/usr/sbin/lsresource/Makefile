# @(#)25        1.1  src/bos/usr/sbin/lsresource/Makefile, cmdcfg, bos411, 9438A411a 9/14/94 10:37:29
# COMPONENT_NAME:
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

VPATH   = ${MAKETOP}/bos/usr/ccs/lib/libcfg/${TARGET_MACHINE}

PROGRAMS        = lsresource
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST             = lsresource
IDIR		  = /usr/sbin/

INCFLAGS  = -I${MAKETOP}/bos/usr/ccs/lib/libcfg/${TARGET_MACHINE}

OFILES          = pt.o lsresource.o

LIBS		  = -ls -lodm -lcfg

.include <${RULES_MK}>

