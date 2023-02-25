# @(#)06        1.2  src/bos/usr/sbin/savebase/Makefile, cfgmethods, bos411, 9428A410j 10/13/93 10:56:51
#
# COMPONENT_NAME: CMDCFG
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS	= savebase 
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

ILIST		= savebase
IDIR		= /usr/sbin/
INCFLAGS	= -I${MAKETOP}/bos/usr/sbin/lvm/include -I${MAKETOP}/bos/usr/sbin/mkboot

LIBS		= -lodm -lcfg

.include <${RULES_MK}>
