# @(#)70	1.5  src/tcpip/usr/ccs/lib/libtcp_ocs/Makefile, tcpip, tcpip41J, 9523A_all 5/31/95 14:38:13
# COMPONENT_NAME: TCPIP - OCS Utility library
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1992
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES		= libtcp_ocs.a
MSGHDRS                 = libtcp_ocs_msg.h
CATFILES                = libtcp_ocs.cat
HFILES                  = ${MSGHDRS}

EXPLIB_TARGETS		= export_libtcp_ocs.a

#ILIST			= libtcp_ocs.a
IDIR			= /usr/ccs/lib/
#IMODE			= 644

CFLAGS			= -D_CSECURITY -D_BSD=44
OFILES			= ocs_vtsup.o ocsvhost.o

.include <${RULES_MK}>
