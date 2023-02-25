# @(#)76	1.3  src/rspc/usr/lib/methods/startpm/Makefile, pwrsysx, rspc41J, 9510A 3/8/95 04:22:06
# COMPONENT_NAME: PWRSYSX
#
# FUNCTIONS: Makefile
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

PROGRAMS	= startpm
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

OFILES		= startpm.o

#CFLAGS		= -DCFGDEBUG
INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common

ILIST		= ${PROGRAMS}
IDIR		= /usr/lib/methods/

LIBS		= -lodm -lcfg

.include <${RULES_MK}>
