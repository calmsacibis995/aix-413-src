# @(#)42        1.3  src/bos/usr/lib/methods/cfggiodev/Makefile, inputdd, bos411, 9428A410j 11/30/93 08:41:31
# COMPONENT_NAME: INPUTDD
#
# FUNCTIONS: N/A
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

VPATH       = ../common

PROGRAMS    = cfggiodev

OFILES      = cfgdevice.o cfgtools.o chkslot.o cfggiodev.o 

INCFLAGS    = -I../common

ILIST       = cfggiodev
IDIR        = /usr/lib/methods/


LIBS        = -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
