# @(#)58        1.7  src/rspc/kernext/pm/pmmd/6030/Makefile, pwrsysx, rspc41J, 9518A_all 4/25/95 00:12:47
#
# COMPONENT_NAME: PWRSYSX
#
# FUNCTIONS:
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT           = pmmd_6030 pmmd_6030pin
IDIR                 = /usr/lib/drivers/
ILIST                = ${KERNEL_EXT}

pmmd_6030_ENTRYPOINT = pmmdconfig
pmmd_6030_IMPORTS    = -bI:../../pmmi/uxpmmipin.exp -bI:pmmd_6030pin.exp
pmmd_6030_EXPORTS    =

pmmd_6030pin_IMPORTS = -bI:../../pmmi/uxpmmipin.exp 
pmmd_6030pin_EXPORTS = -bE:pmmd_6030pin.exp

LIBS                 = ${KERNEXT_LIBS}

pmmd_6030_OFILES     = pmmdconfig.o planar.o slave.o
pmmd_6030pin_OFILES  = planarpin.o slavepin.o pmmddata.o batctl.o   pmcore.o\
		       pmmiif.o    pmmdmisc.o pmtime.o   cpuoff.o	    \
		       pdcarera.o  pdeagle.o  pdkmc.o    pdpcibus.o pdrtc.o \
		       pdsystem.o  pdcommon.o pdeeprom.o pdmemctl.o pdpic.o \
	   	       pdserial.o  pdumcu.o   pdcooper.o pdidaho.o  pdmouse.o \
 		       pdpit.o     pdsio.o    pdv7310.o  pdintfc.o            \
 		       pdpos.o     pdsuprio.o pdwdc24.o  pdppcfnc.o           \
		       pdpower.o   pdcpu.o        			      \
                       dumpregs.o  getregs.o\
		       shpcrfly.o  shpcrsus.o

STRIP_FLAG ?= -s
CFLAGS = -D_KERNEL -D_KERNSYS
#CFLAGS = -qcpluscmt -D_KERNEL -D_KERNSYS -DPM_DEBUG
#CFLAGS = -qcpluscmt -D_KERNEL -D_KERNSYS -DPM_DEBUG  -qlist 
ASFLAGS    += -m 603

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
