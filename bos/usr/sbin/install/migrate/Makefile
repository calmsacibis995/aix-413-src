# @(#)15        1.7  src/bos/usr/sbin/install/migrate/Makefile, cmdinstl, bos41J, 9519B_all 5/11/95 10:40:30
#-----------------------------------------------------------------------
#
# COMPONENT_NAME: (CMDINSTL) LPP Install
#
# FUNCTIONS: Makefile (migrate_32_to_41)
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------------

PROGRAMS        = migrate_32_to_41
SCRIPTS         = merge_swvpds convert_swvpd addvpd_product delvpd_product
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

ILIST           = ${PROGRAMS} ${SCRIPTS}

NOUPDATE_LIST   = /usr/lpp/bos/migrate/migrate_32_to_41 \
		  /usr/lpp/bos/migrate/merge_swvpds \
		  /usr/lpp/bos/migrate/merge_41vpds

addvpd_product_IDIR   = /usr/lib/instl/
delvpd_product_IDIR   = /usr/lib/instl/
IDIR                  = /usr/lpp/bos/migrate/

INCFLAGS        = -I../messages -I../include

OFILES            = migrate_32_to_41.o \
                   vpd_free.o \
                   vpdchg.o \
                   vpdinit.o \
                   vpdreslpp.o \
                   vpdadd.o \
                   vpdclass.o \
                   vpdget.o \
                   vpdremote.o \
                   vpdsql.o \
                   replace_quotes.o

LIBS=  -lodm -lc -ls -lPW -lcfg -lIN -linulib

.include <${RULES_MK}>
