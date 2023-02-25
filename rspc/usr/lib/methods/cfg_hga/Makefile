# @(#)39  1.6  src/rspc/usr/lib/methods/cfg_hga/Makefile, pcihga, rspc411, 9430C411a 7/26/94 12:37:51
# Makefile
#
#   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
#
#   FUNCTIONS: cfg_hga
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
VPATH		= ${MAKETOP}/bos/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS        = cfg_hga

OFILES          = cfgcommon.o cfgtools.o cfghga.o ddstools.o \
		  cfg_runtime_br_rspc.o cfg_dvdr.o cfg_get_dvdr.o

INCFLAGS        = -I${MAKETOP}/bos/usr/lib/methods/common \
	        -I${MAKETOP}/rspc/kernext/disp/pcihga/inc \
	        -I${MAKETOP}/bos/usr/lib/methods/cfgcommon

LIBS            = -ls -lodm -lcfg -lrts

ILIST           = cfg_hga

IDIR            = /usr/lib/methods/

.include <${RULES_MK}>
