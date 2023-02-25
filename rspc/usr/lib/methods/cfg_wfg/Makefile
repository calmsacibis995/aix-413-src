# @(#)62	1.3  src/rspc/usr/lib/methods/cfg_wfg/Makefile, pciwfg, rspc41J, 9507C 1/27/95 15:31:21
# Makefile
#
#   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
#
#   FUNCTIONS: cfg_wfg
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
#
VPATH		= ${MAKETOP}/bos/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS        = cfg_wfg

OFILES          = cfgcommon.o cfgtools.o cfgwfg.o ddstools.o \
		  cfg_runtime_br_rspc.o cfg_dvdr.o cfg_get_dvdr.o

CFLAGS          += -DPM_SUPPORT -DPANEL_TYPE

INCFLAGS        = -I${MAKETOP}/bos/usr/lib/methods/common \
	        -I${MAKETOP}/rspc/kernext/disp/pciwfg/inc \
	        -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

LIBS            = -ls -lodm -lcfg -lrts

ILIST           = cfg_wfg

IDIR            = /usr/lib/methods/

.include <${RULES_MK}>
