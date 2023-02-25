# @(#)82    1.4  src/rspc/kernext/isa/isamouse/Makefile, isamouse, rspc411, 9438C411a 9/22/94 14:09:53
# COMPONENT_NAME: INPUTDD
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# Makefile --- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>
.if defined(GS_DEBUG)
CFLAGS +=-DGS_DEBUG_TRACE -DGS_DEBUG
.endif

VPATH              +=:${MAKETOP}/bos/kernext/inputdd/common
INCFLAGS           =-I../isakbd -I${MAKETOP}/bos/kernext/inputdd/inc
CFLAGS             +=-DPM_SUPPORT
STRIP_FLAG         ?=-s
LDFLAGS_KERNEXT    +=${STRIP_FLAG} 
LIBS               = ${KERNEXT_LIBS}

KERNEL_EXT         = mousedd
IDIR               = /usr/lib/drivers/isa/
ILIST              = mousedd

mousedd_ENTRYPOINT = mseconfig

OFILES = \
    isa_msefnspin.o isa_mseintr.o isa_mseconfig.o isa_msefns.o ktsmtrace.o \
    ktsmfnspin.o ktsmfns.o 

.include <${RULES_MK}>


