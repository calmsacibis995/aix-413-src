# @(#)35        1.1  src/rspc/usr/lbin/stok_dd/Makefile, pcitok, rspc41J, 9513A_all 3/24/95 15:13:26
#
#   COMPONENT_NAME: pcitok
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS        = stok_dd 

VPATH           = ${MAKETOP}/rspc/kernext/pci/stok

OFILES		= main.o stok_dmp.o 

CFLAGS		= -D_KERNEL -D_POWER_MP

INCFLAGS        = -I${MAKETOP}rspc/kernext/pci/stok

INCDIRS:=${INCDIRS:S;/usr/include$;/kernel/include;g}

ILIST           = stok_dd 

IDIR		= /usr/lib/ras/dmprtns/

.include <${RULES_MK}>
