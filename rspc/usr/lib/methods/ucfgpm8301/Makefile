# @(#)44	1.3  src/rspc/usr/lib/methods/ucfgpm8301/Makefile, pwrsysx, rspc41J, 9523A_all 6/1/95 18:36:28
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

VPATH           = ${MAKETOP}/bos/usr/lib/methods/ucfgdevice

PROGRAMS        = ucfgpm8301

OFILES		= ucfgdevice.o ucfg_dvdr.o ucfg_gen.o rds_power_off_slot.o

#CFLAGS		= -D_CFG_RDS -DCFGDEBUG
CFLAGS		= -D_CFG_RDS
INCFLAGS 	= -I${MAKETOP}/bos/usr/lib/methods/ucfgdevice \
		-I${MAKETOP}/bos/usr/lib/methods/common

ILIST           = ucfgpm8301
IDIR		= /usr/lib/methods/

ucfgpm8301_IMPORTS	= -bI:../../../../kernext/pm/pmmi/pmmi.exp

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
