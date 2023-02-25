# @(#)14        1.2  src/bos/usr/sbin/install/netls/Makefile, cmdinstl, bos411, 9428A410j 1/10/94 13:37:20
# COMPONENT_NAME: CMDINSTL
#
# FUNCTIONS:
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

PROGRAMS	= get_license_info
MSGHDRS		= 
CATFILES	= 
HFILES		= ${MSGHDRS}

ILIST		= get_license_info
IDIR		= /usr/lib/instl/

INCFLAGS	=-I../messages -I../include

OFILES          = get_license_info.o

LIBS		= -linulib -ls -lcfg -lswvpd -lodm -lPW

.include <${RULES_MK}>
