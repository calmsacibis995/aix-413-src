# @(#)40  1.2  src/rspc/usr/lib/methods/cfgpcivca/Makefile, pcivca, rspc411, 9438C411a 9/24/94 07:26:41
# Makefile
#
#   COMPONENT_NAME: (PCIVCA) PCI Video Capture Adapter Device Driver
#
#   FUNCTIONS: cfgpcivca
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

VPATH           = ${MAKETOP}/bos/usr/lib/methods/common
VPATH           := ${VPATH}:${MAKETOP}/bos/usr/lib/methods/cfgcommon

PROGRAMS        = cfgpcivca 

OFILES          = cfgcommon.o cfgtools.o cfgpcivca.o ddstools.o \
		  cfg_runtime_br_rspc.o cfg_dvdr.o cfg_get_dvdr.o

#
# Uncomment to get debug information or run dbx
#

#CFLAGS += -g -DDEBUG -DDEBUG_BUS -DCFGDEBUG  -DPM_SUPPORT
CFLAGS += -DPM_SUPPORT

INCFLAGS        = -I${MAKETOP}/bos/usr/lib/methods/common \
	        -I${MAKETOP}/rspc/kernext/pci/pcivca/inc  \
		-I${MAKETOP}/bos/usr/lib/methods/cfgcommon

LIBS            = -ls -lodm -lcfg -lrts

ILIST           = cfgpcivca 

IDIR            = /usr/lib/methods/

.include <${RULES_MK}>
