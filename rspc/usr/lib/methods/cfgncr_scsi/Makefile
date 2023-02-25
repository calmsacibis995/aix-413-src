# @(#)06	1.2  src/rspc/usr/lib/methods/cfgncr_scsi/Makefile, cfgmethods, rspc411, 9430C411a 7/26/94 12:38:26
#
#   COMPONENT_NAME: PCISCSI
#
# FUNCTIONS:
#
#   ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}bos/usr/lib/methods/common
VPATH		:= ${VPATH}:${MAKETOP}bos/usr/lib/methods/cfgcommon

PROGRAMS	= cfgncr_scsi

OFILES		=	cfgcommon.o	\
                        cfg_runtime_br_rspc.o \
                        cfg_dvdr.o \
                        cfg_get_dvdr.o \
			cfgtoolsx.o	\
			cfgncr_scsi.o	\
			cfghscsidc.o	\
			put_scsi_id.o	\
			location.o	\
			pvidtools.o	\
			sccheck.o


INCFLAGS	= -I${MAKETOP}bos/usr/lib/methods/common	\
		  -I${MAKETOP}rspc/kernext/pci/ncr810		\
		  -I${MAKETOP}bos/usr/lib/methods/cfgcommon

ILIST		= cfgncr_scsi
IDIR		= /usr/lib/methods/


LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
