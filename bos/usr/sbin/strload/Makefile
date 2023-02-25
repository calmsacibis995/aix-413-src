# @(#)48        1.5  src/bos/usr/sbin/strload/Makefile, cmdpse, bos411, 9434B411a 8/19/94 17:08:36
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/sbin/execerror

PROGRAMS	= strload
MSGHDRS		= strload_msg.h execerr_msg.h
CATFILES	= strload.cat
HFILES		= ${MSGHDRS}

ILIST		= strload pse.conf dlpi.conf xtiso.conf
IDIR		= /usr/sbin/
pse.conf_IDIR	= /etc/
dlpi.conf_IDIR	= /etc/
xtiso.conf_IDIR	= /etc/

LIBS		= -lodm -lcfg -lpse

NOUPDATE_LIST	= \
	/etc/pse.conf	\
	/etc/dlpi.conf	\
	/etc/xtiso.conf

.include <${RULES_MK}>
