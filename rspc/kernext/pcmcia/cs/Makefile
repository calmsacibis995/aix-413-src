# @(#)89	1.4  src/rspc/kernext/pcmcia/cs/Makefile, pcmciaker, rspc41J, 9511A_all 3/14/95 09:27:58
#
#   COMPONENT_NAME: PCMCIAKER
#
#   FUNCTIONS: Makefile
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

CFLAGS		+= -DPM_ENABLE
#CFLAGS		+= -DDEBUG
#CFLAGS         += -qlist

KERNEL_EXT		= pcmciacs pcmciacspin
ILIST			= pcmciacs pcmciacspin pcmciacs.exp
IDIR			= /usr/lib/drivers/
pcmciacs.exp_IDIR	= /usr/lib/

pcmciacs_OFILES         = cs_cfg.o cs.o cslib.o evh_cs.o evh.o
pcmciacs_ENTRYPOINT	= pcmciacs_config
pcmciacs_IMPORTS	= pcmciacspin.exp
pcmciacs_EXPORTS	= pcmciacs.exp evh.exp

pcmciacspin_OFILES      = evh_intr.o cs_intr.o
pcmciacspin_EXPORTS     = pcmciacspin.exp

LIBS		= ${KERNEXT_LIBS}

.include <${RULES_MK}>
