# @(#)41  1.4  src/rspc/usr/lib/methods/cfg_bbl/Makefile, pcibbl, rspc41J, 9516B_all 4/18/95 10:42:08
#
#   COMPONENT_NAME: (pcibbl) PCI Graphics Adapter Configuration
#
#   FUNCTIONS: cfg_bbl
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
VPATH		= ${MAKETOP}/bos/usr/lib/methods/cfg_bbl:${MAKETOP}/bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon
VPATH		:= ${VPATH}:${MAKETOP}/rspc/usr/lib/methods/common

PROGRAMS        = cfgbbl_pci

OFILES          = cfgcommon.o cfgtools.o cfgbbl_pci.o ddstools.o \
		  cfg_runtime_br_rspc.o cfg_dvdr.o cfg_get_dvdr.o assign_dsp.o 


INCFLAGS        = -I${MAKETOP}/bos/kernel \
	        -I${MAKETOP}/bos/usr/lib/methods/common \
	        -I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
	        -I${MAKETOP}/bos/kernext/disp/bbl/inc   

LIBS            = -ls -lodm -lcfg -lrts

ILIST           = cfgbbl_pci

IDIR            = /usr/lib/methods/

.include <${RULES_MK}>
