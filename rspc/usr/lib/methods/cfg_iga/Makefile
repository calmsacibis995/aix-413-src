# @(#)95	1.1  src/rspc/usr/lib/methods/cfg_iga/Makefile, pciiga, rspc411, 9436D411b 9/5/94 17:12:53
#
#   COMPONENT_NAME: (PCIiga) S3 PCI Graphics Adapter Device Driver
#
#   FUNCTIONS: cfg_iga
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

PROGRAMS        = cfg_iga

OFILES          = cfgdevice.o cfgtools.o chkslot.o cfgiga.o ddstools.o

#
# Uncomment to get debug information or run dbx
#
#CFLAGS		+= -g -DDEBUG -DDEBUG_BUS -DCFGDEBUG 
#OPT_LEVEL =

INCFLAGS        = -I${MAKETOP}/bos/usr/lib/methods/common \
	        -I${MAKETOP}/rspc/kernext/disp/pciiga/inc

LIBS            = -ls -lodm -lcfg -lrts

ILIST           = cfg_iga

IDIR            = /usr/lib/methods/

.include <${RULES_MK}>
