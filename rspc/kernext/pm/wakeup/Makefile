# @(#)77        1.4  src/rspc/kernext/pm/wakeup/Makefile, pwrsysx, rspc41J, 9521A_all 5/23/95 10:51:54
#
# COMPONENT_NAME: PWRSYSX
#
# FUNCTIONS: Makefile
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

ILIST		= pmwakeup
IDIR		= /usr/lib/drivers/

OTHERS		= pmwakeup
BINARIES	= yes

OFILES		= hrstart.o hrsetup.o \
		  decompress.o \
		  console.o comport.o \
		  spiosup.o debug.o 

#DEBUG_FLAG	= -DPM_DEBUG
DEBUG_FLAG	=

LDFLAGS		= -D-1 -e__hrstart -bmap:load.map -bnoreorder \
		  -bloadmap:ld.map -bdbg:loadabs

#ASFLAGS		= -l

M4FLAGS 	= ${DEBUG_FLAG}

CFLAGS		= -qlist -qcpluscmt -qarch=ppc ${DEBUG_FLAG}

INCFLAGS	= -I../pmmi/pmdump -I${MAKETOP}/bos/kernel/ml/POWER

CLEANFILES	=  *.map *.lst *.out ${OFILES}
 
.include <${RULES_MK}>

pmwakeup: ${OFILES}
	${RM} -f ${.TARGET} ld.map load.map
	${LD} ${LDFLAGS} -o ${.TARGET} ${OFILES}
