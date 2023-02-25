#  @(#)56        1.1  src/rspc/kernext/isa/dartic_isa/Makefile, dd_artic, rspc41J, 9508A_notx 2/17/95 15:56:17
#  
#  COMPONENT_NAME: dd_artic -- ARTIC Diagnostic Device driver.
#
#  FUNCTIONS: Makefile
#
#  ORIGINS: 27
#
#  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#  combined with the aggregated modules for this product)
#                   SOURCE MATERIALS
#  (C) COPYRIGHT International Business Machines Corp. 1992, 1993
#  All Rights Reserved
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT      = isadartic 
ILIST           = isadartic 
IDIR		= /usr/lib/drivers/isa/

LIBS            = ${KERNEXT_LIBS}

isadartic_ENTRYPOINT = darticconfig_isa

isadartic_OFILES = \
                config_ric_isa.o \
                globals_ric_isa.o \
                misc_ric_isa.o \
                open_ric_isa.o \
                close_ric_isa.o \
                ioctl_ric_isa.o \
                intr_ric_isa.o


.include <${RULES_MK}>
  
