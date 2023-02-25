# @(#)82        1.5  src/rspc/kernext/pm/pmmi/Makefile, pwrsysx, rspc41J 6/6/95 23:54:14
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

VPATH           +=:./pmdump

KERNEL_EXT	= pmmi pmmipin
IDIR		= /usr/lib/drivers/
ILIST		= ${KERNEL_EXT}
INCFLAGS        = -I./pmdump -I${MAKETOP}bos/usr/sbin/lvm/include \
		  -I${MAKETOP}/rspc/usr/lib/boot/softros/aixmon
#OPT_LEVEL	=


pmmi_ENTRYPOINT	= pm_core_init
pmmi_IMPORTS	= -bI:pmmipin.exp -bI:pm.exp
pmmi_EXPORTS	= -bE:pmmi.exp -bE:uxpmmipin.exp

pmmipin_IMPORTS	= -bI:pm.exp
pmmipin_EXPORTS	= -bE:pmmipin.exp

LIBS		= ${KERNEXT_LIBS}

pmmi_OFILES	= coreinit.o    event.o       parameter.o   state.o  	\
		  pmsyscall.o   pmmiinit.o    pmresidual.o  pci.o
pmmipin_OFILES	= core.o 	transit.o     broadcast.o   pmevent.o   \
  		  pmmidata.o    pcipin.o				\
                  compress.o pmdump.o pmhiber.o savewakeup.o 		\
                  getresidual.o pmhlv.o x41head.o 			\
                  pmdumptools.o bitmap.o compresspin.o pmdumppin.o copyreal.o


STRIP_FLAG ?= -s
#CFLAGS =  -qcpluscmt -D_KERNEL -D_KERNSYS -DPM_DEBUG
CFLAGS =  -qcpluscmt -D_KERNEL -D_KERNSYS

LDFLAGS_KERNEXT += ${STRIP_FLAG}

.include <${RULES_MK}>
