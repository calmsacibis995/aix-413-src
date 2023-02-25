# @(#)22  1.3  src/rspc/usr/lib/methods/cfg_gga/Makefile, pcigga, rspc41J, 9516B_all 4/18/95 10:14:35
#
#   COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
#
#   FUNCTIONS: cfg_gga
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
VPATH           := ${VPATH}:${MAKETOP}/rspc/usr/lib/methods/common

PROGRAMS        = cfg_gga

OFILES          = cfgcommon.o cfgtools.o cfggga.o ddstools.o assign_dsp.o \
		  cfg_runtime_br_rspc.o cfg_gga_dvdr.o cfg_get_dvdr.o

INCFLAGS	= -I${MAKETOP}/bos/usr/lib/methods/common \
		-I${MAKETOP}/bos/usr/lib/methods/cfgcommon \
	        -I${MAKETOP}/rspc/kernext/disp/pcigga/inc 

#CFLAGS         += -g -DDEBUG -DDEBUG_BUS -DCFGDEBUG

LIBS            = -ls -lodm -lcfg -lrts

ILIST           = cfg_gga

IDIR            = /usr/lib/methods/

.include <${RULES_MK}>
