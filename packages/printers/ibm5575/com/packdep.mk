# @(#)55        1.6  src/packages/printers/ibm5575/com/packdep.mk, pkgdevprint, pkg411, GOLD410 2/24/94 12:02:06
#
# COMPONENT_NAME: pkgdevprint
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

USR_LIBLPP_LIST		+= printers.ibm5575.com.pre_d

USR_ODMADD_LIST		+= printer.ibm5575.diag.add%printers.ibm5575.com \
			   printer.ibm5575.add%printers.ibm5575.com
