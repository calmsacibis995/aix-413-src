# @(#)79  1.8  src/rspc/kernext/pci/pcivca/Makefile, pcivca, rspc41J, 9521A_all 5/21/95 21:43:34
# Makefile
#
#   COMPONENT_NAME: (PCIVCA) PCI Video Capture Adapter Device Driver
#
#   FUNCTIONS:
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

#EXPINC_SUBDIRS          = inc
KERNEL_EXT              = vcadd vcadd_pin
IDIR                    = /usr/lib/drivers/pci/
ILIST                   = vcadd vcadd_pin
LIBS                    = ${KERNEXT_LIBS}
#CFLAGS                  += -DVCADBG 
CFLAGS                  += -DPM_SUPPORT -DPANEL_PROTECTION
#			-g -qcheck -qextchk -qattr -qlist -qlistopt     \
#			-qnoro -qsource -qxref  -qflag=I:I

vcadd_ENTRYPOINT        = vca_config
vcadd_pin_ENTRYPOINT    = vca_intr
vcadd_IMPORTS           = -bI:vcadd_pin.exp -bI:wfgdd.imp 
#vcadd_EXPORTS           = -bE:vcadd.exp

vcadd_pin_IMPORTS       = -bI:wfgdd.imp
vcadd_pin_EXPORTS       = -bE:vcadd_pin.exp

vcadd_pin_OFILES        =               \
	                vcadd_pin.o      

vcadd_OFILES            =               \
	                vcadd.o 

INCFLAGS = -I./inc 

.include <${RULES_MK}>
