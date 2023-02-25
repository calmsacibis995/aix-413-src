# @(#)74    1.5  src/rspc/kernext/isa/isakbd/Makefile, isakbd, rspc41J, 9519A_all 5/9/95 07:50:06
#
#   COMPONENT_NAME: isakbd
#
#   FUNCTIONS: defined
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


.include <${MAKETOP}bos/kernext/Kernext.mk>
.if defined(GS_DEBUG)
CFLAGS +=-DGS_DEBUG_TRACE -DGS_DEBUG
.endif

VPATH             +=:${MAKETOP}/bos/kernext/inputdd/common:${MAKETOP}/bos/kernext/inputdd/kbddd:
INCFLAGS          =-I${MAKETOP}/bos/kernext/inputdd/inc
CFLAGS            +=-DPM_SUPPORT -DDIRTY_PATCH
STRIP_FLAG        ?=-s
LDFLAGS_KERNEXT   +=${STRIP_FLAG}
LIBS              = ${KERNEXT_LIBS}

KERNEL_EXT        = kbddd
IDIR              = /usr/lib/drivers/isa/
ILIST             = kbddd

kbddd_ENTRYPOINT  = kbdconfig

OFILES = \
    isa_kbdfnspin.o isa_kbdintr.o isa_kbdconfig.o kbdfns.o ktsmtrace.o \
    ktsmfnspin.o ktsmfns.o keyfns.o keyfnspin.o 

.include <${RULES_MK}>
