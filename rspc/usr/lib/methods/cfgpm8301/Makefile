# @(#)78	1.2  src/rspc/usr/lib/methods/cfgpm8301/Makefile, pwrsysx, rspc41J, 9515B_all 4/14/95 10:54:31
#
# COMPONENT_NAME: PWRSYSX
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH           = ${MAKETOP}/bos/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS        = cfgpm8301

OFILES          = cfgcommon.o cfg_dvdr.o cfg_runtime_br_rspc.o \
			cfg_no_child.o cfg_no_vpd.o cfgtoolsx.o

#CFLAGS		= -DCFGDEBUG
INCFLAGS 	= -I${MAKETOP}/bos/usr/lib/methods/common \
		-I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
		-I${MAKETOP}/rspc/kernext/pm/pmmd/IBM8301

ILIST           = cfgpm8301
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg

.include <${RULES_MK}>
