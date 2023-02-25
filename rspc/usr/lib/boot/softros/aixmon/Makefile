# @(#)89	1.9  src/rspc/usr/lib/boot/softros/aixmon/Makefile, rspc_softros, rspc41J, 9516A_all 4/18/95 16:38:49
#
#   COMPONENT_NAME: RSPC_SOFTROS
#
#   FUNCTIONS: none
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

ILIST		= aixmon
IDIR		= /usr/lib/boot/

OTHERS		= aixmon
BINARIES	= yes

OFILES		=  start.o crtc.o aixmon.o aix_util.o gencmd.o mcmnd.o \
		   load_booti.o iplcb_init.o esck.o nv_utils.o \
		   residual.o res_debug.o

OFILES_V	=  version.o

OFILES_ALL	= ${OFILES} ${OFILES_V}

INCFLAGS	= -I${MAKETOP}/rspc/usr/lib/boot/softros/include \
		  -I${MAKETOP}/rspc/usr/lib/boot/softros/roslib  \
		  -I${MAKETOP}/rspc/usr/lib/boot/mkmboot

LIBDIRS		= -L${EXPORTBASE}/usr/lib/masters

LIBS		= -lsrstand -lsrlibc -lros

LDFLAGS		= -T0x5000 -D-1 -e__start -bmap:load.map  -bnoreorder \
		  -bloadmap:ld.map -bdbg:loadabs

ASFLAGS		= -l`${RM} -f as.lst`as.lst

CFLAGS		= -q`${RM} -f ${.PREFIX}.lst`list -qcpluscmt -DUSE_POLLING -qarch=ppc -D_CHARMODE \
		  -DPM_SUPPORT

CLEANFILES	=  *.map *.lst *.out ${OFILES}

.include <${RULES_MK}>

LIB_DEPEND	=  ${EXPORTBASE}/usr/lib/masters/libsrstand.a  \
		   ${EXPORTBASE}/usr/lib/masters/libsrlibc.a   \
		   ${EXPORTBASE}/usr/lib/masters/libros.a

aixmon:	${OFILES_ALL} ${LIB_DEPEND}
	${RM} -f ld.map
	${RM} -f load.map
	${LD} ${LDFLAGS} -o ${.TARGET}  ${OFILES_ALL}  ${LIBDIRS} ${LIBS}


version.o	: ${OFILES} version.c Makefile version.h

version.h	: ${OFILES} version.c Makefile
	${RM} ${_RMFLAGS_} ${.TARGET}
	${ECHO} "strcpy(rostime,\"`${DATE} +%y%m%d%\H%M`\");" > version.h

