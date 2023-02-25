# @(#)69        1.3  src/bos/usr/sbin/nimconfig/Makefile, cmdnim, bos41J, 9517B_all 4/28/95 14:22:39
#
#   COMPONENT_NAME: CMDNIM
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= nimconfig
OPT_LEVEL	=

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/


CFLAGS		= -D_BSD -g -qdbxextra
LDFLAGS         += -s

nimconfig_OFILE 		= nimconfig.o 

LIBS		= -lmstr -lnim -lodm

.include <${RULES_MK}>
