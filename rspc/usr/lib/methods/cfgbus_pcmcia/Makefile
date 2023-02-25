# @(#)71  1.3  src/rspc/usr/lib/methods/cfgbus_pcmcia/Makefile, rspccfg, rspc41J, 9513A_all 3/28/95 16:09:07
#
#   COMPONENT_NAME: RSPCCFG
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994,1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/lib/methods/cfgcommon
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/rspc/usr/lib/methods/cfgbus_pci

PROGRAMS	= cfgbus_pcmcia

OFILES          = bcm.o cfgbus_pcmcia.o \
		  cfg_runtime_br_rspc.o cfg_dvdr.o \
		  cfg_get_dvdr.o cfgtoolsx.o

INCFLAGS        += -I${MAKETOP}/bos/usr/lib/methods/common \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
		   -I${MAKETOP}/bos/usr/lib/methods/cfgbus \
		   -I${MAKETOP}/rspc/kernext/pcmcia/ssisa \
		   -I${MAKETOP}/rspc/kernext/pcmcia/cs


#CFLAGS          = -DCFGDEBUG -DMK_SP_FILE

ILIST		= cfgbus_pcmcia
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
