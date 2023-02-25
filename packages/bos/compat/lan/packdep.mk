# @(#)90 1.2 src/packages/bos/compat/lan/packdep.mk, sysxcie, bos412, GOLDA411a 5/9/94 10:37:19
#
# COMPONENT_NAME: (SYSXCIE) COMIO Emulator
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST contains the list of files that need to be added to
#               the respective liblpp.a other than the copyright, .al,
#               .tcb, .size, and .inventory files.  For example, any
#               config scripts would be included here.
#               The variable may be left blank if there are no additional
#               files, but the option will exist.
#

# Set environment variable to use objclassdb with unique key identifiers
# because of conflict of PdAt entries with lan device packages
OBJCLASSDB = ${ODE_TOOLS}/usr/lib/uniqueobjclassdb

USR_LIBLPP_LIST         +=

USR_ODMADD_LIST         += sm_ciefddi.add%bos.compat.lan \
                           sm_cietok.add%bos.compat.lan  \
                           sm_cieent.add%bos.compat.lan  \
			   cieent.add%bos.compat.lan     \
			   cietok.add%bos.compat.lan     \
			   ciefddi.add%bos.compat.lan
