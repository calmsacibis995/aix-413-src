# @(#)34	1.4  src/rspc/usr/lib/boot/softros/roslib/Makefile, rspc_softros, rspc41J, 9515A_all 4/11/95 15:07:01
#
#   COMPONENT_NAME: rspc_softros
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES       = libros.a
EXPLIB_TARGETS  = export_libros.a
IDIR            = /usr/lib/masters/

VPATH		= ${MAKETOP}/softros/ros


OFILES          = scsi_api.o scsi_ctl.o scsi_ncr.o scsi_util.o misc_sub.o \
		  misc_asm.o io_sub.o scsi_compat.o disasm.o nvram.o \
		  cmos_sub.o keyboard.o \
		  diskette.o siosetup.o dskt_compat.o ktsm_compat.o uart.o

INCFLAGS	= -I${MAKETOP}/softros/include \
		  -I${MAKETOP}/softros/roslib \
		  -I${MAKETOP}/softros/ros \
		  -I${MAKETOP}/rspc/usr/lib/boot/softros/aixmon

CFLAGS          = -DUSE_POLLING  -qcpluscmt -qarch=ppc -D_CHARMODE


.include <${RULES_MK}>

