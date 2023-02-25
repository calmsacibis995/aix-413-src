# @(#)92	1.8  src/tenplus/tools/fc/Makefile, tenplus, tenplus411, GOLD410 10/27/93 11:43:03
#
# COMPONENT_NAME: (TENPLUS) INed Editor
#
# FUNCTIONS:
#
# ORIGINS:  27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS = fc
fc_OFILES = fc.o common.o box.o

LIBS = -lwin -lutil -lshort -lobj
LIBFLAGS = -L${MAKETOP}/tenplus/lib/window -L${MAKETOP}/tenplus/lib/util -L${MAKETOP}/tenplus/lib/short -L${MAKETOP}/tenplus/lib/obj
INCFLAGS = -I${MAKETOP}/tenplus/include
CFLAGS = -DVBLSCR

.include <${RULES_MK}>
